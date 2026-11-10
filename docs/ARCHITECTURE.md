# Synthetic tissue v0.1

This implementation materializes the user-supplied, previously uncompiled Fable proposal (`original-proposal.txt`). That file is historical input, not validation evidence. The initial upstream revision was `188c5e9`; it contained READMEs and an MIT license, with no executable model.

## State and update order

A 2D lattice stores at most one cell per voxel. Each voxel has glucose, oxygen and waste concentrations, a byte-valued cell state, energy and damage. Structure-of-arrays storage uses approximately 57 bytes/voxel including diffusion and division scratch, excluding allocator/runtime overhead. All quantities and time are arbitrary simulation units, with grid spacing fixed at one.

Each step applies scheduled events, exchanges the left boundary with a reservoir, metabolizes, resolves death/clearance/division, then diffuses. The update order is part of the model and introduces splitting error. There is no mechanics, migration, learned controller, patient initialization or organ coupling.

Diffusion uses the four-neighbor explicit flux stencil with reflecting/no-flux edges. `D*dt <= 0.25` ensures nonnegative stencil weights. Reservoir exchange is separately bounded by `k_exchange*dt <= 1`. Internal diffusion conserves each species; the ledger tracks supply (signed, including outflow), consumption and production. This is a **species bookkeeping test**, not a complete atomic or thermodynamic conservation model. Energy is capped, and excess yield is discarded.

Metabolism uses saturating uptake, oxygen-limited oxidative yield and fermentation of the remaining glucose. Waste causes damage; repair decreases damage. Death occurs at zero energy or damage >= 1. Eligible parents propose one empty neighbor, and each target selects a single proposer using a deterministic hash. Lattice edges and available space influence outcomes.

## ADR 001: small standalone kernel

Use C++20 and Python's standard library so the initial model can be built and audited with minimal dependencies. This supports testing the proposal; it does not establish superiority over existing biological simulators. A future solver must be independently cross-validated before biological claims.

## ADR 002: deterministic execution

Random draws are functions of seed, step, voxel and stream. Metabolism and ledger summation stay serial to avoid thread-dependent reduction rounding; optional OpenMP parallelizes only diffusion. No fast-math flags are enabled. Reproducibility is promised only for the same binary and platform. The printed hash covers step, cells and concentrations, **not** parameters/events/ledger; tests additionally compare serialized checkpoint bytes.

## ADR 003: checkpoint v2

Serialize named parameter fields, step, ledger, named events and every voxel as locale-independent decimal text with 17 significant digits. No structure padding or parameter byte offsets are persisted. The parser validates fields and loads into a temporary organism, so failed loads preserve the old state. Version 1 raw checkpoints are rejected. The representation is architecture-independent; cross-platform bitwise simulation equivalence has not been tested.

Resume forbids simultaneous config/overrides to prevent silently changing the meaning of persisted time and state. For a perturbation study, schedule events before initialization. Event times must lie on the timestep grid; events at time t are applied at the start of the step beginning at t. Seeding changes after initialization have no effect.

Checkpoint files have no checksum or cryptographic authenticity and writes are not crash-atomic. Keep a previous checkpoint if interruption during save is possible. Grid and event counts are bounded, but a deliberately huge valid file can still require substantial memory.
