#!/usr/bin/env python3
"""Controlled engine benchmark; this is not an ASIO/DPC or DAW listening test."""
import argparse,json,statistics,subprocess,platform
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--before',type=Path,required=True);p.add_argument('--after',type=Path,required=True);p.add_argument('--out',type=Path,required=True);p.add_argument('--repeats',type=int,default=3);a=p.parse_args();runs={'before':[],'after':[]};a.out.mkdir(parents=True,exist_ok=True)
for repeat in range(a.repeats):
 for name in (['before','after'] if repeat%2==0 else ['after','before']):
  command=[str(getattr(a,name))];
  if repeat==0:command.append(str(a.out/f'predelay-{name}.f32'))
  result=subprocess.run(command,check=True,capture_output=True,text=True);(a.out/f'{name}-{repeat}.txt').write_text(result.stdout);rows=[]
  for line in result.stdout.splitlines():
   if line.startswith('BENCH '):
    sr,mod,cpu,p99,worst,energy=map(float,line.split()[1:]);rows.append(dict(sr=int(sr),modulated=bool(mod),cpu_percent=cpu,p99_block_us=p99,max_block_us=worst,energy=energy))
  assert len(rows)==8,'Incomplete benchmark';runs[name].append(rows)
summary=[]
for i in range(8):
 before=statistics.median(r[i]['cpu_percent'] for r in runs['before']);after=statistics.median(r[i]['cpu_percent'] for r in runs['after']);b=runs['before'][0][i];c=runs['after'][0][i];assert abs(b['energy']-c['energy'])<=max(1,abs(b['energy']))*1e-5,'Steady tone changed unexpectedly'
 summary.append(dict(sample_rate=c['sr'],modulated=c['modulated'],before_cpu_percent=before,after_cpu_percent=after,reduction_percent=100*(1-after/before),before_p99_us=statistics.median(r[i]['p99_block_us'] for r in runs['before']),after_p99_us=statistics.median(r[i]['p99_block_us'] for r in runs['after'])))
(a.out/'benchmark.json').write_text(json.dumps(dict(platform=platform.platform(),baseline_commit='e6b647b748eac85aefa16d31f83259d04ffee449',repeats=a.repeats,block_size=64,summaries=summary,runs=runs),indent=2)+'\n');print(json.dumps(summary,indent=2))
