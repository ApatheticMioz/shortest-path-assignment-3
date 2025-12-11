# Shortest Path Algorithms Benchmark

C++ implementations of classic shortest-path algorithms (Dijkstra, Bellman-Ford, Floyd-Warshall, Johnson) with interactive solving and automated benchmarking/visualization for sparse, dense, and mixed graphs. Includes LaTeX report assets and generated DOT graphs for quick visualization.

## Key Features
- Implements four shortest-path algorithms with relaxation counting and simple memory estimators.
- Random graph generator for sparse, dense, and mixed (optionally negative) graphs; exports Graphviz DOT files (`viz_*.dot`).
- Interactive mode that reads a graph from `input.txt`, supports directed/undirected graphs, and prints per-algorithm results.
- Benchmark mode (500 runs per graph type) that records timing/relaxation statistics to `statistics_report.txt`.
- LaTeX report (`report.tex`) with pseudocode, analysis tables, and discussion.

## Tech Stack
- C++17 (STL)
- Graphviz (optional, for rendering DOT files)
- LaTeX (for the included report)

## Project Structure
- `i232523_Source_Asst-3.cpp` — interactive driver: read a graph, run all algorithms, or run a small experiment suite.
- `solution-test.cpp` — benchmark harness: 500-run statistics per graph type, writes `statistics_report.txt`.
- `input.txt` — user-provided graph input for interactive mode (created if missing).
- `viz_*.dot` — generated DOT graphs for Sparse/Dense/Mixed sample runs.
- `images/` — screenshots referenced by the LaTeX report.
- `report.tex` — assignment report.

## Build
Use g++ (or any C++17 compiler).

```bash
# Interactive solver / experiment runner
 g++ -std=c++17 -O2 i232523_Source_Asst-3.cpp -o shortest_path

# Benchmark harness
 g++ -std=c++17 -O2 solution-test.cpp -o benchmark
```

## Usage

### Interactive solver (menu)
1) Prepare `input.txt` with:
```
V E
u v w
u v w
...
```
2) Run `shortest_path`, choose option `1`, confirm directed (1) or undirected (0) when prompted.
3) The program prints distances for each algorithm; Johnson reports negative cycles as needed.

### Experimental analysis (menu option 2)
- Runs randomized sparse/dense/mixed graphs, outputs timing/relaxations table per config, and saves `viz_Sparse.dot`, `viz_Dense.dot`, `viz_Mixed.dot` for visualization.

### Benchmark harness
- Run `benchmark` to execute 500 iterations per graph type; results are appended to `statistics_report.txt` with mean, median, std dev, and 95% CI for time and relaxations.

## Visualizing DOT graphs
Use https://dreampuf.github.io/GraphvizOnline/ or `dot -Tpng viz_Sparse.dot -o sparse.png` (Graphviz required).

## Notes
- Dijkstra is skipped when negative edges are present.
- Johnson’s algorithm detects negative cycles and aborts the run in that case.
- Default graph sizes: sparse/mixed draw V∈[10,50]; dense draws V∈[100,200].
