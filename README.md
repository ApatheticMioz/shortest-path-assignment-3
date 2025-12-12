# Shortest Path Algorithms Benchmark

C++ implementations of classic shortest-path algorithms (Dijkstra, Bellman-Ford, Floyd-Warshall, Johnson) with interactive solving and automated benchmarking for sparse, dense, and mixed graphs.

**Status:** Archived / Refactored

## Description

This project provides optimized implementations of four fundamental shortest-path algorithms:

- **Dijkstra's Algorithm** — Single-source shortest path for non-negative weights, O((V+E) log V)
- **Bellman-Ford Algorithm** — Handles negative weights with cycle detection, O(VE)
- **Floyd-Warshall Algorithm** — All-pairs shortest paths, O(V³)
- **Johnson's Algorithm** — Efficient all-pairs for sparse graphs with negative weights, O(V² log V + VE)

Each algorithm includes relaxation counting, memory estimation, and performance benchmarking capabilities.

## Project Structure

```
.
├── shortest_path.cpp   # Interactive solver and experimental analysis
├── benchmark.cpp       # Statistical benchmark harness (500 iterations)
├── input.txt           # Sample graph input file
├── LICENSE             # MIT License
├── CONTRIBUTING.md     # Contribution guidelines
├── CHANGELOG.md        # Version history
└── README.md           # This file
```

## Requirements

- C++17 compatible compiler (g++, clang++)
- Graphviz (optional, for DOT file visualization)

## Installation

Clone the repository and build using any C++17 compiler:

```bash
git clone https://github.com/ApatheticMioz/shortest-path-assignment-3.git
cd shortest-path-assignment-3
```

## Build

```bash
# Build interactive solver
g++ -std=c++17 -O2 shortest_path.cpp -o shortest_path

# Build benchmark harness
g++ -std=c++17 -O2 benchmark.cpp -o benchmark
```

## Usage

### Interactive Solver

1. Prepare `input.txt` with your graph:
   ```
   V E
   u v w
   u v w
   ...
   ```
   Where V = vertices, E = edges, and each edge is defined as (source, destination, weight).

2. Run the solver:
   ```bash
   ./shortest_path
   ```

3. Choose from the menu:
   - **Option 1:** Solve graph from `input.txt`
   - **Option 2:** Run experimental analysis with random graphs

### Benchmark Harness

Run statistical benchmarks (500 iterations per graph type):

```bash
./benchmark
```

Results are saved to `statistics_report.txt` with mean, median, standard deviation, and 95% confidence intervals.

### Visualizing Graphs

Generated DOT files can be visualized using:
- Online: https://dreampuf.github.io/GraphvizOnline/
- Command line: `dot -Tpng viz_Sparse.dot -o sparse.png`

## Notes

- Dijkstra's algorithm is automatically skipped when negative edge weights are detected
- Johnson's algorithm reports negative cycles when detected
- Graph sizes: Sparse/Mixed use V∈[10,50]; Dense uses V∈[100,200]

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
