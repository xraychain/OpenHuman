import csv
import os
from pathlib import Path
import subprocess
import sys
import tempfile

binary = str(Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory() as directory:
    root = Path(directory)
    def run(*args, ok=True, threads=1):
        result = subprocess.run([binary, *map(str,args)], cwd=root, text=True, capture_output=True,
                                env={**os.environ, 'OMP_NUM_THREADS':str(threads)}, timeout=30)
        assert (result.returncode == 0) == ok, (args, result.stderr)
        return result.stdout
    def info(output):
        return dict(line.split('=',1) for line in output.splitlines())
    for args in [('--steps','2bad'),('--set','nx=-1'),('--set','maintenance=-1'),('--set','dt=nan'),
                 ('--set','Vg=inf'),('--set','seed=-1'),('--every','0'),('--unknown',),('--steps',)]:
        run(*args,ok=False)
    common=['--set','nx=12','--set','ny=8','--set','event=2 supply_glucose 0.25']
    full=info(run(*common,'--steps',57,'--save','full.ckpt','--metrics','full.csv','--every',10))
    run(*common,'--steps',13,'--save','split.ckpt')
    resumed=info(run('--resume','split.ckpt','--steps',44,'--save','resumed.ckpt'))
    assert full['hash']==resumed['hash']
    assert (root/'full.ckpt').read_bytes()==(root/'resumed.ckpt').read_bytes()
    other=info(run(*common,'--steps',57,'--save','threaded.ckpt',threads=4))
    assert full['hash']==other['hash']
    assert (root/'full.ckpt').read_bytes()==(root/'threaded.ckpt').read_bytes()
    with (root/'full.csv').open() as f:
        rows=list(csv.DictReader(f))
    assert [int(r['step']) for r in rows]==[0,10,20,30,40,50,57]
    run('--resume','full.ckpt','--set','dt=.01',ok=False)
    run('--resume','full.ckpt','--config','missing.cfg',ok=False)
    valid=(root/'full.ckpt').read_text()
    for text in [valid[:100],valid+'garbage',valid.replace('OHCK 2','OHCK 1'),valid.replace('supply_glucose 0.25','nx 0.25')]:
        (root/'bad.ckpt').write_text(text)
        run('--resume','bad.ckpt',ok=False)
print('CLI, process restart, corruption and thread-count checks passed')
