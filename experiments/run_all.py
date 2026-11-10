#!/usr/bin/env python3
"""OpenHuman v0.1 experiments A-F plus sweeps. Usage: python experiments/run_all.py build/openhuman"""
import argparse, csv, json, os, pathlib, platform, statistics, subprocess, sys, hashlib
from datetime import datetime, timezone
os.environ.setdefault("OMP_NUM_THREADS", "1")

parser = argparse.ArgumentParser()
parser.add_argument("binary", nargs="?", default="build/openhuman")
parser.add_argument("--quick", action="store_true", help="CI smoke only; not the full research protocol")
parser.add_argument("--output", default="results")
args = parser.parse_args()
BIN = str(pathlib.Path(args.binary).resolve())
OUT = pathlib.Path(args.output); OUT.mkdir(parents=True, exist_ok=True)
STEPS = 300 if args.quick else 30000  # 3000 time units at dt = 0.1

def run(name, steps=STEPS, sets=(), extra=()):
    m = OUT / f"{name}.csv"
    cmd = [BIN, "--steps", str(steps), "--every", "100", "--metrics", str(m)]
    for s in sets:
        cmd += ["--set", s]
    out = subprocess.run(cmd + list(extra), check=True, capture_output=True, text=True).stdout
    info = dict(line.split("=", 1) for line in out.split())
    with m.open() as stream:
        rows = list(csv.DictReader(stream))
    (OUT / f"{name}.stdout.txt").write_text(out)
    (OUT / f"{name}.command.json").write_text(json.dumps(cmd + list(extra), indent=2))
    alive = [int(r["alive"]) for r in rows]
    tail = alive[len(alive) // 2:]
    return {
        "wall_s": float(info["wall_s"]), "steps_per_s": float(info["steps_per_s"]),
        "hash": info["hash"], "final_alive": alive[-1],
        "tail_mean": statistics.mean(tail), "tail_min": min(tail), "tail_max": max(tail),
        "max_abs_residual": max(abs(float(r[k])) for r in rows
                                for k in ("resid_glucose", "resid_oxygen", "resid_waste")),
        "min_conc": min(float(r["min_conc"]) for r in rows),
    }

R = {"metadata": {"protocol": "smoke" if args.quick else "full", "steps": STEPS, "platform": platform.platform(), "python": platform.python_version(), "utc": datetime.now(timezone.utc).isoformat(), "binary_sha256": hashlib.sha256(pathlib.Path(BIN).read_bytes()).hexdigest(), "omp_threads": os.environ["OMP_NUM_THREADS"]}}
R["A_baseline"] = run("A_baseline", extra=["--snapshot", str(OUT / "A_snapshot.csv")])
R["B_glucose"] = {g: run(f"B_glucose_{g}", sets=[f"supply_glucose={g}"]) for g in (5, 2.5, 1, 0.5, 0.25)}
R["C_oxygen"] = {o: run(f"C_oxygen_{o}", sets=[f"supply_oxygen={o}"]) for o in (1, 0.3, 0.1, 0.03, 0)}
R["D_recovery"] = run("D_recovery", sets=[f"event={STEPS * 0.1 / 3:.10g} supply_glucose 0.25", f"event={STEPS * 0.1 / 2:.10g} supply_glucose 5"])

ck = str(OUT / "E.ckpt")
run("E_part1", steps=STEPS // 2, extra=["--save", ck])
e2 = run("E_part2", steps=STEPS - STEPS // 2, extra=["--resume", ck])  # separate process
R["E_persistence"] = {"resumed_hash": e2["hash"], "uninterrupted_hash": R["A_baseline"]["hash"],
                      "identical": e2["hash"] == R["A_baseline"]["hash"]}

R["F_reproducibility"] = {
    "same_seed_identical": run("F_repeat")["hash"] == R["A_baseline"]["hash"],
    "tail_mean_by_seed": {s: run(f"F_seed_{s}", sets=[f"seed={s}"])["tail_mean"] for s in range(2, 7)},
}
R["sweep_maintenance"] = {v: run(f"S_maint_{v}", sets=[f"maintenance={v}"]) for v in (0.01, 0.02, 0.03, 0.05, 0.08)}
R["sweep_glyco_boost"] = {v: run(f"S_boost_{v}", sets=[f"glyco_boost={v}"]) for v in (0, 1, 2)}

json.dump(R, open(OUT / "summary.json", "w"), indent=2)
print(json.dumps(R, indent=2))

assert R["E_persistence"]["identical"], "checkpoint continuation mismatch"
assert R["F_reproducibility"]["same_seed_identical"], "repeat mismatch"
def verify(value):
    if isinstance(value, dict):
        if "min_conc" in value:
            assert value["min_conc"] >= 0, "negative concentration"
            assert value["max_abs_residual"] < 1e-6, "conservation failure"
        for child in value.values(): verify(child)
verify(R)
