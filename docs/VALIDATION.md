# Validation boundaries and test plan

`ctest --test-dir build --output-on-failure` runs C++ invariant tests and a Python subprocess integration suite. Checks use explicit failures, so Release/NDEBUG builds still enforce them.

- Diffusion: exact corner impulse after one step, constant-field invariance, discrete no-flux cosine eigenmode, nonnegative concentrations and closed-domain mass conservation.
- Biology rules: division, starvation, toxicity, damage repair and population event accounting.
- Persistence: round-trip and continued trajectory, byte-identical checkpoint after separate-process restart, rejection of truncation/version errors/invalid event names, and transactional failed load.
- Input handling: negative, nonfinite, overflowing and trailing-junk numbers; invalid rates, unstable diffusion, forbidden and off-grid events; incompatible resume options.
- Output: initial and final metrics are present even if the final step is not a reporting multiple.
- Reproducibility: same-seed state hash, complete serialized state and thread-count comparison. A serial build ignores thread counts; the Linux OpenMP CI job exercises actual parallel diffusion.

The full experiment driver runs 30,000 steps per trajectory (except the two halves of the persistence test), checks repeat/continuation hashes, minimum concentration >= 0 and absolute species residual < 1e-6. This tolerance applies to the default 96x48 protocol, not arbitrary grid sizes. `--quick` runs 300 steps and proportionally earlier perturbations; it is only a CI smoke test.

Local Release CMake/CTest verification passed both suites (2/2), using CMake 3.22.1 from the existing Android SDK and Apple Clang 21.0.0. CMake Debug with ASan/UBSan passed both suites (2/2), and direct sanitizer builds also passed. The smoke experiment driver passed.

Local evidence is recorded in `results/reference/` and `RESEARCH_FINDINGS.md`. CI is configured for Linux/macOS Release and ASan/UBSan, plus Linux OpenMP. Workflow configuration is not evidence that remote CI has run. No GitHub authentication was available during this implementation.

Not established: convergence of the complete coupled model under timestep refinement, spatial/grid-size convergence, independence from the finite lattice boundary, agreement with another simulator, calibration against biology, human predictive validity, or cross-compiler bitwise reproducibility. The analytic diffusion test validates one component only.
