#!/usr/bin/env python3
"""Measure sparse and initially full 1M-voxel lattices in separate child processes."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import resource
import subprocess
import time

parser=argparse.ArgumentParser()
parser.add_argument('binary')
parser.add_argument('--output',default='results/benchmark.json')
args=parser.parse_args()
binary=str(Path(args.binary).resolve())
records=[]
for label, extra in [('sparse',[]),('full',['--set','seed_depth=1000','--set','seed_fraction=1'])]:
    command=[binary,'--set','nx=1000','--set','ny=1000','--steps','200','--every','200',*extra]
    begin=time.perf_counter()
    result=subprocess.run(command,check=True,text=True,capture_output=True,env={**os.environ,'OMP_NUM_THREADS':'1'})
    elapsed=time.perf_counter()-begin
    # ru_maxrss is the largest child high-water mark so far; records explicitly
    # label this cumulative maximum (not a per-case allocation measurement).
    rss=resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss
    if platform.system()!='Darwin': rss*=1024
    records.append({'case':label,'command':command,'elapsed_process_s':elapsed,
                    'children_peak_rss_bytes_so_far':rss,'stdout':dict(line.split('=',1) for line in result.stdout.splitlines())})
output={'platform':platform.platform(),'binary_sha256':hashlib.sha256(Path(binary).read_bytes()).hexdigest(),
        'omp_threads':1,'records':records,'note':'1M voxels; full seeds 1M live cells initially. Memory is cumulative child peak RSS, includes runtime.'}
Path(args.output).parent.mkdir(parents=True,exist_ok=True)
Path(args.output).write_text(json.dumps(output,indent=2)+'\n')
print(json.dumps(output,indent=2))
