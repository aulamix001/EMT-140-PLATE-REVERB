#!/usr/bin/env python3
"""48 kHz wet-only IR metrics. Objective contrasts are not listening validation."""
import argparse,csv,itertools,json,subprocess
from pathlib import Path
import numpy as np
from scipy.signal import butter,sosfilt
P=argparse.ArgumentParser();P.add_argument('--ir-dir',type=Path,required=True);P.add_argument('--bank',type=Path,required=True);P.add_argument('--out',type=Path,required=True);P.add_argument('--check',action='store_true');P.add_argument('--renderer',type=Path);args=P.parse_args()
if args.renderer:subprocess.run([str(args.renderer),str(args.ir_dir)],check=True)
bank=json.loads(args.bank.read_text())['presets'];sr=48000
manifest=args.ir_dir/'rendered-parameters.csv'
if args.renderer:assert manifest.exists(),'renderer did not write its compiled parameter manifest'
if manifest.exists():
 with manifest.open() as f:rendered=list(csv.DictReader(f))
 assert len(rendered)==len(bank),'stale renderer bank count'
 for expected,actual in zip(bank,rendered):
  assert int(actual['number'])==expected['number']
  for id,value in expected['parameters'].items():assert abs(float(actual[id])-value)<1e-5,f'stale render: {expected["name"]} {id}'

sos=butter(3,[354,707],fs=sr,btype='bandpass',output='sos');rows=[]
for preset in bank:
 x=np.fromfile(args.ir_dir/f"{preset['number']}.f32",dtype='<f4').reshape(-1,2).astype(np.float64);assert np.isfinite(x).all()
 energy=np.sum(x*x,axis=1);peak=float(energy.max());assert peak>1e-16
 onset=int(np.flatnonzero(energy>=peak*1e-6)[0])/sr*1000
 y=sosfilt(sos,x,axis=0);edc=np.cumsum(np.sum(y*y,axis=1)[::-1])[::-1];db=10*np.log10(np.maximum(edc,1e-300)/edc[0]);mask=(db < -5)&(db > -35)
 t=np.arange(len(y))/sr;slope,offset=np.polyfit(t[mask],db[mask],1);rt=-60/slope;fit=slope*t[mask]+offset;r2=1-np.sum((db[mask]-fit)**2)/np.sum((db[mask]-np.mean(db[mask]))**2)
 spectrum=np.sum(np.abs(np.fft.rfft(x,axis=0))**2,axis=1);freq=np.fft.rfftfreq(len(x),1/sr)
 def band(a,b):return float(spectrum[(freq>=a)&(freq<b)].sum())
 mid=band(300,2000);bright=10*np.log10(band(3000,12000)/mid);bass=10*np.log10(band(20,180)/mid)
 m=(x[:,0]+x[:,1])/2;s=(x[:,0]-x[:,1])/2;width=np.sqrt(np.dot(s,s)/max(1e-30,np.dot(m,m)))
 broad_edc=np.cumsum(energy[::-1])[::-1];broad_db=10*np.log10(np.maximum(broad_edc,1e-300)/broad_edc[0]);broad_mask=(broad_db<-5)&(broad_db>-35);broad_slope,broad_offset=np.polyfit(t[broad_mask],broad_db[broad_mask],1);broad_rt=-60/broad_slope
 density=float(np.mean(energy[int(onset*sr/1000):int(onset*sr/1000)+480]>peak*1e-4))
 row=dict(number=preset['number'],name=preset['name'],category=preset['category'],rt60_s=round(float(rt),5),rt60_broadband_s=round(float(broad_rt),5),onset_ms=round(onset,4),intrinsic_onset_ms=round(onset-preset['parameters']['predelay'],4),brightness_db=round(float(bright),4),bass_db=round(float(bass),4),width=round(float(width),5),decay_fit_r2=round(float(r2),5),density_first_10ms=round(density,5))
 rows.append(row)
def distance(a,b):
 return max(abs(np.log(a['rt60_s']/b['rt60_s']))/np.log(1.15),abs(a['onset_ms']-b['onset_ms'])/8,abs(a['brightness_db']-b['brightness_db'])/1.5,abs(a['bass_db']-b['bass_db'])/1.5,abs(a['width']-b['width'])/.08)
within=[];cross=[];all_within=[]
for a,b in itertools.combinations(rows,2):
 if a['category']==b['category']:
  d=distance(a,b);all_within.append(float(d))
  if d<1:within.append(dict(a=a['name'],b=b['name'],distance=round(float(d),4)))
 elif distance(a,b)<1:cross.append(dict(a=a['name'],b=b['name'],distance=round(float(distance(a,b)),4)))
categories=sorted({r['category'] for r in rows});centres=[]
for category in categories:
 group=[r for r in rows if r['category']==category];centre={k:float(np.median([r[k] for r in group])) for k in ['rt60_s','onset_ms','brightness_db','bass_db','width']};centre['name']=centre['category']=category;centres.append(centre)
between=[dict(a=a['name'],b=b['name'],distance=round(float(distance(a,b)),4)) for a,b in itertools.combinations(centres,2)]
failures=[]
if cross:failures.append(f'{len(cross)} cross-category pairs lack minimum contrast')
if within:failures.append(f'{len(within)} within-category pairs lack minimum contrast')
weak=[p for p in between if p['distance']<1]
if weak:failures.append(f'{len(weak)} category centroids lack minimum contrast')
for r in rows:
 if r['density_first_10ms']<.7:failures.append(f"Sparse first 10 ms: {r['name']}")
 if not .25<r['rt60_s']<9 or r['decay_fit_r2']<.9:failures.append(f"Unreliable decay: {r['name']}")
 if r['intrinsic_onset_ms']>2 or r['intrinsic_onset_ms']<-.1:failures.append(f"Late/invalid intrinsic onset: {r['name']}")
args.out.mkdir(parents=True,exist_ok=True)
(args.out/'metrics.json').write_text(json.dumps(dict(sample_rate=sr,wet_only=True,metrics=rows,within_failures=within,cross_category_failures=cross,minimum_within_distance=min(all_within),category_distances=between,failures=failures),indent=2)+'\n')
with (args.out/'metrics.csv').open('w',newline='') as f:
 w=csv.DictWriter(f,fieldnames=list(rows[0]),lineterminator='\n');w.writeheader();w.writerows(rows)
print(json.dumps(dict(presets=len(rows),within_failures=within,cross_category_failures=cross,weak_categories=weak,onset_range=[min(r['intrinsic_onset_ms'] for r in rows),max(r['intrinsic_onset_ms'] for r in rows)],rt60_range=[min(r['rt60_s'] for r in rows),max(r['rt60_s'] for r in rows)],other_failures=[s for s in failures if not s.startswith(('Late/invalid','100 '))][:12]),indent=2))
if args.check and failures:raise SystemExit(1)
