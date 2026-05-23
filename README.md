# Smoke Simulation

A grid-based fluid and smoke simulation written in Objective-C++ and Metal. An older version of the project written for the CPU, using the Raylib framework, is also included.

## Build and Run
```bash
make
./bin/smoke_gpu
```

### CPU Version
```bash
cd CPU_version
make
./smoke_cpu
```

## Controls
- **Left Click**: Drag to apply velocity to the fluid.
- **Right Click**: Click/Drag to emit smoke into the grid.
- **Scroll Wheel**: Adjust brush radius.
- **Space**: Pause/Resume the simulation.
- **R**: Reset the simulation.
- **1**: Toggle Smoke visualization (Default).
- **2**: Toggle Speed visualization.
- **3**: Toggle Divergence (Pressure Error) visualization.
- **B**: Toggle Solid Mode (Not in CPU version).
- **W**: Toggle left side Wind and Smoke Emitter (Not in CPU version) (Default off)


---

## Technical Details
I followed the [course notes](https://www.cs.ubc.ca/~rbridson/fluidsimulation/fluids_notes.pdf) from the University of British Columbia on Fluid Simulation to learn the algorithms for this simulation.

This project solves [Navier-Stokes equations for incompressible fluids](https://en.wikipedia.org/wiki/Navier%E2%80%93Stokes_equations) ($a = -\frac{\nabla p}{\rho}, \nabla \cdot u = 0$) using the projection method on a [staggered MAC grid](https://en.wikipedia.org/wiki/Staggered_grid).

The pressure Poisson equation is solved using [Gauss-Seidel](https://en.wikipedia.org/wiki/Gauss%E2%80%93Seidel_method) iteration. To accelerate convergence, [Successive Over-Relaxation](https://en.wikipedia.org/wiki/Successive_over-relaxation) (SOR) is applied to each iteration. The GPU version specifically utilizes [Red-Black Gauss-Seidel](https://en.wikipedia.org/wiki/Gauss%E2%80%93Seidel_method#Parallel_version) to enable parallel execution across the grid. Advection for both velocity and smoke is handled via a [semi-Lagrangian](https://en.wikipedia.org/wiki/Semi-Lagrangian_scheme) scheme with [bilinear interpolation](https://en.wikipedia.org/wiki/Bilinear_interpolation).
