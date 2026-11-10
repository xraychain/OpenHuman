# Evidence-led roadmap

1. **Implemented:** compile the supplied lattice proposal, validate inputs, add portable structured checkpoints, deterministic ledger, tests, CI configuration and reproducible experiment scripts.
2. **Next:** run remote Linux/OpenMP CI; test timestep and domain-size sensitivity; add independent diffusion reference comparisons and crash-safe/checksummed checkpoints.
3. **Investigate current findings:** vary uptake, waste, maintenance and initial field conditions; distinguish stable regulation from resource-limited survival and finite-domain saturation. Use predeclared stationarity/recovery criteria and multi-seed confidence intervals.
4. **Improve numerical model:** assess implicit diffusion, consistent units, quiescence, energetics and mechanics. Preserve baseline fixtures when changing semantics.
5. **Learned-rule study (not implemented):** train a shared local transition surrogate on separately generated trajectories, hold out entire seeds/perturbations, compare multi-step rollout error and conservation against the mechanistic baseline. Treat learned accuracy on synthetic data as surrogate fidelity, not biological validation.
6. **Long-term vision:** couple validated subsystems, model uncertainty and only then explore profile-based initialization. A persistent synthetic tissue is an experimental substrate for OpenHuman, not a completed digital organism or a human model.
