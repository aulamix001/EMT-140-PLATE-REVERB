#!/usr/bin/env python3
import argparse,json,subprocess,sys
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--renderer',type=Path,required=True);p.add_argument('--out',type=Path,required=True);a=p.parse_args();a.out.mkdir(parents=True,exist_ok=True)
bank={'presets':[{'number':i+1,'name':'Plate '+chr(65+i),'category':'Model','parameters':{'predelay':0}} for i in range(3)]};manifest=a.out/'models.json';manifest.write_text(json.dumps(bank))
subprocess.run([str(a.renderer),str(a.out/'ir'),'models'],check=True)
subprocess.run([sys.executable,str(Path(__file__).with_name('measure_presets.py')),'--ir-dir',str(a.out/'ir'),'--bank',str(manifest),'--out',str(a.out),'--check'],check=True)
r=json.loads((a.out/'metrics.json').read_text())['metrics'];assert r[2]['brightness_db']-r[0]['brightness_db']>=3,'C must be at least 3 dB brighter than A';assert r[0]['brightness_db']-r[1]['brightness_db']>=2,'A must be at least 2 dB brighter than B';assert all(1.5<x['rt60_s']<2.5 for x in r),'reference-band decay unstable'
print('PASS: flat A/B/C profiles have distinct measured tonal characters')
