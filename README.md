# Balls Into Bins Simulation

This project simulates balls-into-bins load balancing strategies for a Cisco
interview homework exercise. It compares how well different allocation policies
reduce bin imbalance, and how much algorithmic cost they pay to achieve that
quality.

The main experiment uses 10,000 balls, 16 and 32 bins, and multiple trials per
scenario. Results are exported to CSV and visualized as plots.

## Problem Statement

Given `m` balls and `n` bins, each ball is assigned to one bin. The goal is to
reduce the maximum bin load and the overall imbalance between bins.

Implemented strategies include:

- Random placement: choose one random bin per ball.
- Power of `k` choices: sample `k` bins and place the ball into the least loaded
  sampled bin. `k=2` is the classic Power of Two Choices.
- Absolute minimum: inspect all bins and place the ball into the current least
  loaded bin. In this project this is represented as Power of `k` with `k=n`.
- Stateful Round Robin: place balls cyclically using a small amount of state.
- Heap-based policies: maintain a tracked set of low-load bins using a heap.

The simulator also supports:

- Weighted balls, where ball weights are sampled from `1.0` to `10.0`.
- Randomized initial bin state, where bins may start with synthetic pre-existing
  load.

## Theoretical Motivation

With one random choice, the maximum load can be much larger than the average
load; asymptotically, the classic bound is around `log n / log log n` above the
average scale. The Power of Two Choices is interesting because adding only one
extra random candidate dramatically reduces imbalance. Larger `k` values improve
load quality further, but with diminishing returns and higher per-ball cost.

This project explores that engineering tradeoff directly: better balance versus
more random draws, load checks, comparisons, or maintained state.

## Repository Structure

```text
.
├── Makefile
├── README.md
├── include/balls_bins/        # Public C++ headers
├── src/                       # Simulator implementations and runners
│   ├── main.cpp               # Full experiment entry point
│   ├── SingleRunner.cpp       # Compile-time configured single experiment
│   ├── ExperimentRunner.cpp   # Scenario definitions and CSV export
│   ├── PowerKSimulator.cpp
│   ├── StatefulRoundRobinSimulator.cpp
│   └── HeapSizeSPowerOfKSimulator.cpp
├── scripts/
│   ├── plot_results.py        # Regenerates plots from results CSV
│   └── create_presentation.py
├── results/                   # Generated CSV outputs
├── plots/                     # Generated PNG plots
└── presentation/              # Presentation artifacts
```

## Build Instructions

The project uses a Makefile and requires a C++17 compiler.

```bash
make
```

This builds:

- `build/balls_bins_sim` for the full experiment suite.
- `build/single_runner` for the compile-time configured single experiment.

To remove compiled binaries:

```bash
make clean
```

## Run the Full Experiment

Run all configured scenarios and regenerate all plots:

```bash
make all-results
```

This runs `build/balls_bins_sim`, writes:

```text
results/simulation_results.csv
```

and then runs:

```bash
python3 scripts/plot_results.py
```

to regenerate the PNG files in `plots/`.

You can also run only the simulation:

```bash
make results
```

or only regenerate plots from an existing CSV:

```bash
make plots
```

## Run a Single Experiment

The single-experiment runner is configured in code, not through CLI flags. Edit
the `kConfig` constant in `src/SingleRunner.cpp`, rebuild, and run:

```bash
make single
```

The current checked-in single-run configuration is:

- `balls = 10000`
- `bins = 32`
- `trials = 10`
- `model = PowerK`
- `k = 2`
- `weighted_balls = true`
- `random_initialization = false`
- `workload_seed = 42`
- `allocation_seed = 1337`

It writes:

```text
results/single_simulation_results.csv
```

For example, to run 10,000 balls into 16 bins with unweighted Power of Two
Choices, set these fields in `src/SingleRunner.cpp`:

```cpp
int balls = 10000;
int bins = 16;
int k = 2;
bool weighted_balls = false;
bool random_initialization = false;
```

Then run:

```bash
make single
```

## Experiment Modes

The default experiment suite is defined in `src/ExperimentRunner.cpp`.

Main dimensions:

- Bin count: `16` and `32`.
- Number of balls: `10000`.
- Trials per scenario: `10`.
- Strategy: random, Power-`k`, absolute minimum, Round Robin, and heap variants.
- Weighted balls: enabled or disabled.
- Random initialization: enabled or disabled.

The main comparison and tradeoff experiments combine weighted balls and random
initialization into four modes:

- `unweighted_empty`: unweighted balls, no initialization.
- `weighted_empty`: weighted balls, no initialization.
- `unweighted_initialized`: unweighted balls, randomized initial bin loads.
- `weighted_initialized`: weighted balls, randomized initial bin loads.

Randomized initialization uses initial bin loads from `0` to `100`. Weighted
balls use weights from `1.0` to `10.0`.

## Plots and Outputs

Simulation results are saved in:

```text
results/simulation_results.csv
```

Plots are saved in:

```text
plots/
```

Most plots show a load-quality metric in the top panel and cost per ball in the
bottom panel, or show cost-quality tradeoff scatter plots. Important metrics
include:

- `max_load_mean`: average maximum load across trials.
- `cv_load_mean`: coefficient of variation of bin loads.
- `cost_per_ball_mean`: average modeled algorithmic cost per ball.

Key plot groups:

- Diagram 1: stateless baseline comparison.
- Diagram 2: diminishing returns as `k` increases.
- Diagram 3: stateful Round Robin versus stateless policies.
- Diagram 4: weighted balls and Round Robin behavior.
- Diagram 5: heap-size sweep.
- Diagrams 6-7: final unweighted and weighted tradeoffs.
- Diagram 8: main policy comparisons for each mode.
- Diagram 9: cost-quality tradeoff plots for each mode.

Diagram 9 includes the combined stress case:

```text
plots/09_tradeoff_weighted_initialized_max_load.png
plots/09_tradeoff_weighted_initialized_cv_load.png
```

These are the "Diagram 9 - Random initialization + weighted balls" plots. They
show the cost-quality tradeoff when bins start with existing random load and
incoming balls also have variable weights. This mode is intended to stress the
algorithms more realistically than the clean unweighted, empty-bin case.

## Cost Model

The simulator compares both balancing quality and the cost paid to get it. The
default cost weights are defined in `src/ExperimentRunner.cpp` and
`src/SingleRunner.cpp`:

```text
random_draw = 1.0
load_read = 3.0
compare = 1.0
state_read = 3.0
state_update = 5.0
state_memory_per_slot = 100.0
heap_update_per_level = 5.0
```

Examples:

- Random placement pays for random draws.
- Power of `k` pays for `k` random draws, bin-load reads, and comparisons.
- Absolute minimum pays more because it checks all bins.
- Round Robin pays for state memory, state reads, and state updates.
- Heap policies pay for maintained state and heap update levels.

The exact cost breakdown is exported to the CSV alongside the load metrics.

## Reproducibility

The default experiments use fixed seeds:

```text
workload_seed = 42
allocation_seed = 1337
```

Each trial derives deterministic per-trial seeds from those values. Rebuilding
and rerunning the same code should reproduce the same CSV results. If scenario
parameters, compiler/runtime behavior, or seed values are changed, results may
change accordingly.

## How to Read the Results

Random placement is the cheapest strategy, but it usually leaves noticeably
higher imbalance.

Power of Two Choices is the main practical result: with only two sampled bins, it
gets close to the best load balance in many scenarios while keeping the cost far
below global inspection.

Absolute minimum gives the strongest balance because it uses global knowledge of
all bins, but the cost model charges it for that global visibility.

Weighted balls and randomized initial bin loads make the setup harder and more
realistic. They help show whether a policy still behaves well when work items are
not identical and the system does not start perfectly empty.

## Notes / Assumptions

- There are no command-line flags for the C++ runners. Experiment parameters are
  currently configured in source code.
- The weighted reference line is an average-load lower bound, not a computed
  optimal weighted bin-packing solution.
- For initialized bins, the project reports an average-load lower bound rather
  than computing a true optimum for the initialized state.
- Plot generation requires Python packages used by `scripts/plot_results.py`:
  `pandas` and `matplotlib`.
