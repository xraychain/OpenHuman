# Measured research findings — 10 November 2026

## Protocol and provenance

Executed the supplied A–F protocol and both parameter sweeps, 30,000 steps at dt=0.1, grid 96x48, default seed 1. A full run was repeated after input/checkpoint hardening. The retained reference data is the final run. The driver records commands, stdout, binary SHA-256, platform and run timestamp. Source derives from upstream `188c5e9` and the user-supplied proposal; the delivery commit contains the exact source and measurements together.

Machine/platform: `macOS-26.6.2-arm64-arm-64bit`. Compiler: Apple Clang 21.0.0, C++20, `-O3`, serial build. Hardware model/core count was unavailable through the restricted system query, so it is not inferred. Performance is a single-run observation, not an isolated repeated benchmark.

## Results

| Experiment | Observed result |
|---|---|
| A baseline | 925 live cells at t=3000; second-half mean 946.47, range 921–1207 |
| B glucose 5, 2.5, 1, 0.5, 0.25 | Final live counts: 925, 770, 480, 528, 805 |
| C oxygen 1, 0.3, 0.1, 0.03, 0 | Final live counts: 925, 899, 888, 930, 971 |
| D glucose reduction at t=1000, restored at t=1500 | Live counts at t=1000/1500/3000: 1490/1009/937 |
| E restart in a separate process | Resumed/uninterrupted hash equal: True |
| F repeat with same seed | Hash equal: True |
| F seeds 2–6 | Second-half population means range 937.50–942.40 |
| Maintenance 0.01, 0.02, 0.03, 0.05, 0.08 | Final live counts: 930, 929, 925, 688, 414 |
| Glycolytic boost 0, 1, 2 | Final live counts: 937, 925, 926 |

Baseline population at t=1500/2000/2500/3000: 1207/927/929/925. This finite trajectory does not establish homeostasis: no preregistered stationarity criterion, long-horizon/domain-size study or confidence interval has been applied. Recovery also cannot be inferred solely from the final population's closeness to baseline.

All sampled concentrations were nonnegative. The largest absolute species-ledger residual in the full protocol was approximately 7.34e-7 (oxygen-supply 0.3 case); the baseline maximum was 5.58e-09. These passed the protocol's 1e-6 absolute tolerance. Residuals are accumulated floating-point bookkeeping discrepancies; relative error and longer-run drift deserve further study.

## Surprising behavior and limits

The glucose response is non-monotonic, and the zero-oxygen trajectory still supports cells under the programmed fermentation rules. These are findings about this artificial system, not evidence for human physiology. Possible mechanisms include uptake/waste feedback and death opening spatial niches, but causal attribution requires controlled ablations. Initial fields are set to the supply concentration across the **entire grid**, so each glucose/oxygen sweep changes both the initial reservoir and ongoing boundary condition. Interpret these as different environments, not a clean acute nutrient dose-response.

The simulation has no calibrated length or time scale, vascular network, mechanics, immune system, organs, learned rules or biological data fit. It is not a living digital organism or a clinical simulator. The unchanged proposal's biological and literature remarks are unverified historical notes.

## One-million-voxel benchmark

| Initial occupancy | Steps | Core loop seconds | Steps/s | Final live cells |
|---|---:|---:|---:|---:|
| sparse | 200 | 1.3428 | 148.9 | 3695 |
| full | 200 | 2.5122 | 79.6 | 1000000 |

Peak child RSS observed across the two cases: 55.98 MiB. This is a cumulative child-process high-water mark, **not a distinct per-case allocation measurement**. The sparse case begins with a narrow seeded strip; the full case begins with 1,000,000 live cells. Loop timings exclude initialization and final summaries; process elapsed times are retained in the JSON. Concurrent background work and platform effects can alter these numbers. No GPU or actual OpenMP performance was measured locally.

## Access and spending

No paid API calls were made (task expenditure USD 0). The expiring USD 69 credit could not be isolated with a verified hard limit from the protected USD 100. GitHub CLI had no authenticated session, so no push or remote CI result is claimed. See `API_BUDGET.md` and `VALIDATION.md`.
