"""Reproducible offline 500 Hz octave-band Schroeder T30 measurements.
Build RenderImpulse.cpp first; requires NumPy/SciPy. Not a hardware comparison.
"""
import json, subprocess, tempfile
from pathlib import Path
import numpy as np
from scipy.signal import butter, sosfilt
root=Path(__file__).resolve().parents[1]
renderer=Path(tempfile.gettempdir())/'render-plate-ir'
subprocess.run(['g++','-O2','-std=c++17','-I',str(root/'Source'),str(root/'Tests/RenderImpulse.cpp'),'-o',str(renderer)],check=True)
rows=[]
for model in range(3):
 for target in [.5,2.,5.5]:
  with tempfile.NamedTemporaryFile(suffix='.raw') as f:
   subprocess.run([str(renderer),str(model),str(target),f.name],check=True)
   x=np.fromfile(f.name,dtype='float64').reshape(-1,2)
  y=sosfilt(butter(3,[354,707],fs=48000,btype='bandpass',output='sos'),x[:,0])
  e=np.cumsum(y[::-1]**2)[::-1]
  db=10*np.log10(np.maximum(e,1e-300)/e[0]);t=np.arange(len(y))/48000
  mask=(db < -5)&(db > -35);slope,offset=np.polyfit(t[mask],db[mask],1)
  measured=-60/slope;error=(measured/target-1)*100
  row=dict(plate=chr(65+model),target_s=target,measured_s=round(measured,4),error_percent=round(error,2),correlation=round(float(np.corrcoef(x.T)[0,1]),4),pass_8_percent=bool(abs(error)<=8))
  rows.append(row);print(row,flush=True)
(root/'docs/measurements.json').write_text(json.dumps(rows,indent=2)+'\n')
raise SystemExit(0 if all(r['pass_8_percent'] for r in rows) else 1)
