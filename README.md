# OpenHuman

**An open-source research initiative toward a persistent, local-first digital organism.**

[English](README.md) · [Türkçe](README.tr.md)

> **Project status: Experimental synthetic tissue v0.1.** OpenHuman is a research vision, not a working human simulator. No biological predictions or clinical capabilities are claimed.


## Working v0.1 prototype

The supplied C++20 proposal is now a compiled, locally tested 2D synthetic tissue simulator. It is uncalibrated and uses arbitrary units. Only this first experimental substrate of the long-term vision below is implemented.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/openhuman --steps 30000 --metrics baseline.csv --save tissue.ckpt
./build/openhuman --resume tissue.ckpt --steps 1000
python3 experiments/run_all.py build/openhuman
python3 experiments/benchmark.py build/openhuman
```

Requires a C++20 compiler, CMake >= 3.16 and Python 3.9+. Python scripts use only the standard library. OpenMP is opt-in (`-DOPENHUMAN_OPENMP=ON`, requires its development library). `--quick` runs a short CI experiment. Generated outputs default to `results/`.

CLI options: `--config file`, repeated `--set key=value`, `--steps N`, `--every K`, `--metrics file.csv`, `--events file.csv`, `--snapshot file.csv`, `--save file.ckpt`, `--resume file.ckpt`. Config lines are `key = value`, with `#` comments; scheduled perturbations use `event = 1000 supply_glucose 0.25`. Resume cannot be combined with config or overrides. Checkpoints from the original raw-memory draft are unsupported.

Read the [measured findings](docs/RESEARCH_FINDINGS.md), [architecture](docs/ARCHITECTURE.md), [validation boundaries](docs/VALIDATION.md), [roadmap](docs/ROADMAP.md) and [API budget constraints](docs/API_BUDGET.md). CI has been configured but not yet run on GitHub.

## The vision

What if a computer could host a simulated organism that **maintains a changing internal state**, responds to its environment, and continues evolving through simulated time?

OpenHuman aims to explore that question by building the foundations for a **computational organism**: a system in which cells, tissues, organs, and regulatory processes interact over time, rather than a collection of independent calculators that only output predefined health scores.

The long-term ambition is to initialize a virtual organism from a profile—such as age, sex-related physiology, body composition, environment, nutrition, activity, and available health measurements—and study how its simulated state changes under different conditions.

This is **not** a claim that a real person can currently be recreated from medical records. A profile provides incomplete observations; any generated internal state will contain assumptions and uncertainty.

## The core idea: a system that evolves

OpenHuman is intended to investigate several connected capabilities:

- **Organism Genesis:** Generate a plausible starting state from a configurable profile and explicit uncertainty ranges.
- **Persistent state:** Maintain and checkpoint the simulated organism across time and application restarts.
- **Multiscale biology:** Connect cell-level agents or populations, tissues, organ systems, and their shared environments.
- **Emergent behavior:** Let interactions, resource constraints, and feedback loops drive modeled outcomes instead of scripting a result for each scenario.
- **Homeostasis and adaptation:** Investigate whether simulated systems can regulate energy, oxygen, nutrients, waste, and other modeled variables.
- **Disease and intervention experiments:** Introduce defined perturbations or model plugins and observe their effects across the simulated system.
- **Traceable experiments:** Record events, model versions, input parameters, state changes, and uncertainty so results can be reproduced and inspected.
- **Accelerated simulated time:** Explore faster-than-real-time execution where model fidelity and available compute permit it.

Cancer is **one possible research scenario**, not the foundation of the project. The primary subject is the simulated organism itself.

## Research principles

1. **Biology before appearance.** A convincing 3D body is not evidence of a realistic biological model.
2. **Shared rules, individual states.** Millions of simulated agents may use the same learned or mechanistic rules while keeping distinct local states. Model parameters and simulation-state memory are different costs.
3. **Multiple scales, explicit approximations.** A full human body cannot currently be modeled cell by cell at molecular detail on a typical personal computer. Resolution must be selected according to the scientific question.
4. **Physical and biological constraints.** Learned models must be checked against conservation laws, known physiology, and experimental observations.
5. **Uncertainty is a first-class output.** Similar profiles need not correspond to identical organisms or identical futures.
6. **Reproducibility over spectacle.** Every experiment should record its seeds, inputs, model versions, assumptions, and evaluation metrics.
7. **Local-first by design.** Simulation and sensitive data should remain on the user's machine by default. Any future sharing of appropriately de-identified results must be explicit and opt-in.
8. **Validation is mandatory.** An internally consistent simulation does not establish clinical predictive accuracy.

## Proposed architecture

```text
                 Human profile / optional observations
                                |
                                v
                     Organism Genesis Engine
                                |
                                v
                  Persistent Organism State
                                |
           +--------------------+--------------------+
           |                    |                    |
           v                    v                    v
      Cell / agent          Tissue and          Regulation and
       processes          organ networks         environment
           |                    |                    |
           +--------------------+--------------------+
                                |
                                v
                     Simulation Runtime
                   (local CPU / GPU execution)
                                |
                  +-------------+-------------+
                  |                           |
                  v                           v
        Checkpoints and logs         Experiment analysis
                                      (local by default)
                                              |
                                              v
                                  Optional, opt-in sharing
                                  of de-identified results
```

The diagram is a **proposal**, not an implemented architecture. Machine learning may support selected components, but an LLM alone is not a validated model of human physiology.

## Roadmap

The table below describes the long-term research program. Initial work on stages 0–2 and 6 is implemented for the uncalibrated lattice prototype; these milestones are not biologically validated.

| Stage | Research milestone | Example success criterion |
| --- | --- | --- |
| 0 | Define model boundaries, data interfaces, and evidence standards | Published technical specification and test plan |
| 1 | Build a small synthetic tissue model | Stable, reproducible interactions among nutrients, energy use, waste, growth, and cell death |
| 2 | Add feedback and perturbation experiments | Measurable recovery or failure modes under controlled conditions |
| 3 | Couple multiple biological subsystems | Conservation and interface tests across model boundaries |
| 4 | Prototype profile-based genesis | Generate plausible ensembles, not a falsely precise single patient |
| 5 | Compare outputs with independent published or appropriately governed data | Documented calibration, uncertainty, and out-of-sample errors |
| 6 | Optimize local execution and time acceleration | Published speed-versus-fidelity benchmarks |

A future goal is to test whether **months or years of simulated time can run within hours or days** on consumer hardware for selected model resolutions. This is an engineering target, **not a performance or accuracy promise**.

## What OpenHuman is not

- Not a medical device, diagnostic tool, or clinical decision-support system.
- Not a digital copy of an identifiable person.
- Not a proven method to predict an individual's disease course, survival, or treatment response.
- Not a claim that emergent computational behavior is biological life or consciousness.
- Not a replacement for clinical trials, medical professionals, or experimental validation.

**Do not use OpenHuman outputs to make medical decisions.** If future research uses patient data, it must follow applicable consent, privacy, ethical-review, and legal requirements.

## Getting involved

The first lattice prototype is available. Contributions are most useful in the form of:

- Relevant peer-reviewed research and existing open-source simulation frameworks.
- Testable model designs, benchmarks, and validation datasets with appropriate usage rights.
- Critiques of feasibility, scaling, biological assumptions, and architecture.

Run the tests before proposing changes, and include measured evidence for model or performance claims.

## License

Distributed under the [MIT License](LICENSE).

---

*OpenHuman is an ambitious long-term research direction. The first objective is modest and testable: create a small simulated biological system that maintains state, responds to perturbations, and can be evaluated against evidence.*
