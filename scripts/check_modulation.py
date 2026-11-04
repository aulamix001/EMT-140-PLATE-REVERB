#!/usr/bin/env python3
import argparse,json,subprocess
from pathlib import Path
import numpy as np
p=argparse.ArgumentParser();p.add_argument('--renderer',type=Path,required=True);p.add_argument('--out',type=Path,required=True);a=p.parse_args();a.out.mkdir(parents=True,exist_ok=True);subprocess.run([str(a.renderer),str(a.out)],check=True)
ratios=[]
for name in ['stationary','modulated']:
 x=np.fromfile(a.out/f'{name}.f32',dtype='<f4').reshape(-1,2).astype(float);f=np.fft.rfftfreq(len(x),1/48000);s=np.sum(abs(np.fft.rfft(x*np.hanning(len(x))[:,None],axis=0))**2,axis=1)
 carrier=s[abs(f-1000)<.12].sum();side=s[(abs(f-1000)>.2)&(abs(f-1000)<12)].sum();ratios.append(float(side/carrier))
result=dict(sample_rate=48000,tone_hz=1000,rate_hz=.8,depth=10,stationary_sideband_ratio=ratios[0],modulated_sideband_ratio=ratios[1]);(a.out/'modulation.json').write_text(json.dumps(result,indent=2)+'\n')
assert ratios[0]<1e-6,'stationary tank developed sidebands'
assert ratios[1]>.05,'delay modulation sidebands must exceed 5% of carrier energy'
print('PASS: stationary sideband ratio',ratios[0],'modulated',ratios[1])
