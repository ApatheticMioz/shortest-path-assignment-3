/*
 * Name: Muhammad Abdullah Ali
 * Roll Num: i23-2523
 * Section: 5A
 * Department: Department of Data Science
 * Course: Design and Analysis of Algorithms
 * Assignment 3: Shortest Path Algorithms
 */

#include <iostream>
#include <vector>
#include <queue>
#include <tuple>
#include <algorithm>
#include <limits>
#include <chrono>
#include <random>
#include <iomanip>
#include <fstream>
#include <string>

using namespace std;

const long long INF = 1e18;

struct Edge {
    int u, v;
    long long w;
};

// Visualization helper
void saveGraphToDOT(const string& filename, int V, const vector<tuple<int,int,int>>& edges) {
    ofstream out(filename);
    out << "digraph G {\n  rankdir=LR;\n  node [shape=circle];\n";
    int limit = (edges.size() > 200) ? 200 : edges.size();
    for(int i=0; i<limit; ++i) {
        auto [u, v, w] = edges[i];
        out << "  " << u << " -> " << v << " [label=\"" << w << "\"];\n";
    }
    out << "}\n";
    out.close();
    cout << "   -> Saved visualization to '" << filename << "'\n";
}

class Graph {
    int V;
    vector<Edge> edges;
    vector<vector<pair<int, long long>>> adj;
    vector<vector<long long>> matrix;
    bool isDirected;

public:
    long long relaxationCount; 

    Graph(int vertices, bool directed = true) {
        V = vertices;
        isDirected = directed;
        adj.resize(V + 1);
        matrix.resize(V + 1, vector<long long>(V + 1, INF));
        for (int i = 1; i <= V; ++i) matrix[i][i] = 0;
        relaxationCount = 0;
    }

    void addEdge(int u, int v, long long w) {
        edges.push_back({u, v, w});
        adj[u].push_back({v, w});
        if (w < matrix[u][v]) matrix[u][v] = w;

        if (!isDirected) {
            edges.push_back({v, u, w});
            adj[v].push_back({u, w});
            if (w < matrix[v][u]) matrix[v][u] = w;
        }
    }

    void printStructure() {
        cout << "\n" << string(50, '=') << "\n LOADED GRAPH STRUCTURE (V=" << V << ")\n" << string(50, '=') << "\n";
        for (int i = 1; i <= V; ++i) {
            if (adj[i].empty()) {
                cout << "  Vertex " << i << " -> (No outgoing edges)\n";
            } else {
                cout << "  Vertex " << i << " connects to:\n";
                for (const auto& edge : adj[i]) {
                    cout << "    -> Vertex " << edge.first << " (Weight: " << edge.second << ")\n";
                }
            }
        }
        cout << string(50, '=') << "\n";
    }

    // Memory usage estimator
    double getMemoryUsage(string algoType, int E_count) {
        double bytes = 0;
        if (algoType == "Floyd-Warshall") {
            bytes += (double)V * V * sizeof(long long) * 2; 
        } else {
            bytes += (double)V * sizeof(vector<pair<int, long long>>);
            bytes += (double)E_count * sizeof(pair<int, long long>);
            bytes += (double)V * sizeof(long long);
            if (algoType == "Johnson") {
                bytes += (double)V * sizeof(long long); 
                bytes += (double)E_count * sizeof(Edge);
            }
        }
        return bytes / 1024.0; 
    }

    bool relax(int u, int v, long long w, vector<long long>& dist) {
        relaxationCount++; 
        if (dist[u] != INF && dist[u] + w < dist[v]) {
            dist[v] = dist[u] + w;
            return true;
        }
        return false;
    }

    // Algorithm 1: Dijkstra
    vector<long long> dijkstra(int src) {
        relaxationCount = 0;
        vector<long long> dist(V + 1, INF);
        priority_queue<pair<long long, int>, vector<pair<long long, int>>, greater<pair<long long, int>>> pq;

        dist[src] = 0;
        pq.push({0, src});

        while (!pq.empty()) {
            long long d = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            if (d > dist[u]) continue;

            for (auto& edge : adj[u]) {
                int v = edge.first;
                long long weight = edge.second;
                if (relax(u, v, weight, dist)) {
                    pq.push({dist[v], v});
                }
            }
        }
        return dist;
    }

    // Algorithm 2: Bellman-Ford
    pair<bool, vector<long long>> bellmanFord(int src) {
        relaxationCount = 0;
        vector<long long> dist(V + 1, INF);
        dist[src] = 0;
        bool changed = false;

        // Relax edges V-1 times
        for (int i = 1; i <= V - 1; ++i) {
            changed = false;
            for (const auto& e : edges) {
                if (relax(e.u, e.v, e.w, dist)) changed = true;
            }
            if (!changed) break;
        }

        // Check for Negative Cycles
        for (const auto& e : edges) {
            if (dist[e.u] != INF && dist[e.u] + e.w < dist[e.v]) {
                return {true, {}}; 
            }
        }
        return {false, dist};
    }

    // Algorithm 3: Floyd-Warshall
    vector<vector<long long>> floydWarshall() {
        relaxationCount = 0;
        vector<vector<long long>> dist = matrix;

        for (int k = 1; k <= V; ++k) {
            for (int i = 1; i <= V; ++i) {
                for (int j = 1; j <= V; ++j) {
                    relaxationCount++; 
                    if (dist[i][k] != INF && dist[k][j] != INF) {
                        if (dist[i][k] + dist[k][j] < dist[i][j]) {
                            dist[i][j] = dist[i][k] + dist[k][j];
                        }
                    }
                }
            }
        }
        return dist;
    }

    // Algorithm 4: Johnson
    vector<vector<long long>> johnson() {
        relaxationCount = 0;
        
        // Add virtual source
        vector<Edge> augmentedEdges = edges;
        for (int i = 1; i <= V; ++i) {
            augmentedEdges.push_back({V + 1, i, 0});
        }

        // Run Bellman-Ford for potentials h[]
        vector<long long> h(V + 2, INF);
        h[V + 1] = 0;
        for (int i = 1; i <= V + 1; ++i) { 
            for (const auto& e : augmentedEdges) {
                relaxationCount++; 
                if (h[e.u] != INF && h[e.u] + e.w < h[e.v]) {
                    h[e.v] = h[e.u] + e.w;
                }
            }
        }

        // Check negative cycle
        for (const auto& e : augmentedEdges) {
            if (h[e.u] != INF && h[e.u] + e.w < h[e.v]) return {}; 
        }

        // Reweight Edges
        vector<vector<pair<int, long long>>> originalAdj = adj; 
        for (int u = 1; u <= V; ++u) {
            for (auto& e : adj[u]) {
                e.second = e.second + h[u] - h[e.first];
            }
        }

        // Run Dijkstra for every vertex
        vector<vector<long long>> allPairsDist(V + 1, vector<long long>(V + 1));
        for (int u = 1; u <= V; ++u) {
            long long currentRelaxCount = relaxationCount;
            vector<long long> d = dijkstra(u); 
            relaxationCount += currentRelaxCount; 

            for (int v = 1; v <= V; ++v) {
                if (d[v] != INF) {
                    allPairsDist[u][v] = d[v] - h[u] + h[v];
                } else {
                    allPairsDist[u][v] = INF;
                }
            }
        }

        // Restore original weights
        adj = originalAdj;
        return allPairsDist;
    }
};

// Graph Generator
void generateConnectedGraph(int V, int E, bool allowNegative, int& outV, int& outE, vector<tuple<int, int, int>>& outEdges) {
    outV = V;
    outE = E;
    outEdges.clear();
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> weights(allowNegative ? -5 : 1, 20);
    vector<int> nodes(V);
    for(int i=0; i<V; ++i) nodes[i] = i + 1;
    shuffle(nodes.begin(), nodes.end(), gen);

    // Ensure connectivity
    for (int i = 1; i < V; ++i) {
        int u = nodes[i];
        uniform_int_distribution<> connectedDist(0, i - 1);
        int v = nodes[connectedDist(gen)];
        outEdges.emplace_back(u, v, weights(gen));
    }

    // Add random edges
    uniform_int_distribution<> nodeDist(1, V);
    int remainingEdges = E - (V - 1);
    if (remainingEdges < 0) remainingEdges = 0;

    for (int i = 0; i < remainingEdges; ++i) {
        int u = nodeDist(gen);
        int v = nodeDist(gen);
        if (u == v) (u < V) ? v = u + 1 : v = u - 1;
        outEdges.emplace_back(u, v, weights(gen));
    }
    outE = outEdges.size();
}

void printRow(string algo, double time, long long relax, double mem, string neg, string comp) {
    cout << left << setw(15) << algo 
         << setw(12) << fixed << setprecision(3) << time 
         << setw(12) << relax
         << setw(12) << setprecision(2) << mem
         << setw(28) << neg 
         << setw(25) << comp << endl;
}

void printTableHeader() {
    cout << left << setw(15) << "Algorithm" << setw(12) << "Time(ms)" << setw(12) << "Relaxations"
         << setw(12) << "Mem(KB)" << setw(28) << "Neg Weights?" << setw(25) << "Complexity" << endl;
    cout << string(104, '-') << endl;
}

void runExperiments() {
    cout << "Running Experimental Analysis...\n";

    struct Config { string type; int V; int E; bool neg; };
    random_device rd;
    mt19937 gen(rd());

    // Experiment Configurations
    vector<Config> configs = {
        {"Sparse", uniform_int_distribution<>(10, 50)(gen), 0, false}, 
        {"Dense", uniform_int_distribution<>(100, 200)(gen), 0, false}, 
        {"Mixed", uniform_int_distribution<>(10, 50)(gen), 0, true} 
    };
    
    // Adjust E based on Type
    configs[0].E = configs[0].V + 5; // Sparse
    configs[1].E = configs[1].V * (configs[1].V - 1) * 0.8; // Dense
    configs[2].E = configs[2].V * 2; // Mixed

    for (auto& cfg : configs) {
        int V, E;
        vector<tuple<int, int, int>> edges;
        generateConnectedGraph(cfg.V, cfg.E, cfg.neg, V, E, edges);
        saveGraphToDOT("viz_" + cfg.type + ".dot", V, edges);

        Graph g(V);
        for(auto [u, v, w] : edges) g.addEdge(u, v, w);

        cout << "\n" << string(104, '=') << endl;
        cout << " ANALYSIS: " << cfg.type << " Graph (V=" << V << ", E=" << E << ")" << endl;
        cout << string(104, '=') << endl;
        printTableHeader();

        // Run Algorithms
        if (!cfg.neg) {
            auto start = chrono::high_resolution_clock::now();
            g.dijkstra(1);
            auto end = chrono::high_resolution_clock::now();
            printRow("Dijkstra", chrono::duration<double, milli>(end - start).count(), g.relaxationCount, g.getMemoryUsage("Dijkstra", E), "No", "O((V+E)logV)");
        } else {
             printRow("Dijkstra", 0.0, 0, 0.0, "No (Skipped)", "O((V+E)logV)");
        }

        auto start = chrono::high_resolution_clock::now();
        g.bellmanFord(1);
        auto end = chrono::high_resolution_clock::now();
        printRow("Bellman-Ford", chrono::duration<double, milli>(end - start).count(), g.relaxationCount, g.getMemoryUsage("Bellman-Ford", E), "Yes", "O(VE)");

        start = chrono::high_resolution_clock::now();
        g.floydWarshall();
        end = chrono::high_resolution_clock::now();
        printRow("Floyd-Warsh", chrono::duration<double, milli>(end - start).count(), g.relaxationCount, g.getMemoryUsage("Floyd-Warshall", E), "Yes", "O(V^3)");

        start = chrono::high_resolution_clock::now();
        g.johnson();
        end = chrono::high_resolution_clock::now();
        printRow("Johnson", chrono::duration<double, milli>(end - start).count(), g.relaxationCount, g.getMemoryUsage("Johnson", E), "Yes", "O(V^2logV + VE)");
        
        cout << string(104, '-') << endl;
    }
    cout << "\n(Tip: Copy text from .dot files to https://dreampuf.github.io/GraphvizOnline/ to view graphs)\n";
}

void solveFromFile() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    // File handling
    ifstream checkFile("input.txt");
    if (checkFile.good()) {
        cout << "\nFound existing 'input.txt'. Using it.\n";
    } else {
        ofstream createFile("input.txt");
        createFile.close();
        cout << "\nCreated 'input.txt'. Please paste your input (V E, then edges) and save.\n";
    }
    checkFile.close();

    cout << "-> Press ENTER once 'input.txt' is ready...";
    cin.get();

    ifstream inFile("input.txt");
    if (!inFile.is_open() || inFile.peek() == ifstream::traits_type::eof()) {
        cout << "Error: Could not open input.txt or file is empty.\n";
        return;
    }

    int V, E;
    inFile >> V >> E;

    cout << "Is the graph Directed? (1 for Yes, 0 for No): ";
    bool isDirected;
    cin >> isDirected;

    Graph g(V, isDirected);
    for (int i = 0; i < E; ++i) {
        int u, v;
        long long w;
        inFile >> u >> v >> w;
        g.addEdge(u, v, w);
    }
    inFile.close();

    g.printStructure();
    cout << "\nProcessing Algorithms...\n";

    // Results
    cout << "\n" << string(30, '-') << "\nAlgorithm: Dijkstra (Src: 1)\n" << string(30, '-') << "\n";
    vector<long long> d_dist = g.dijkstra(1);
    cout << left << setw(10) << "Vertex" << "Distance" << endl;
    for (int i = 1; i <= V; ++i) cout << left << setw(10) << i << (d_dist[i] == INF ? -1 : d_dist[i]) << "\n";

    cout << "\n" << string(30, '-') << "\nAlgorithm: Bellman-Ford (Src: 1)\n" << string(30, '-') << "\n";
    auto bf_res = g.bellmanFord(1);
    if (bf_res.first) {
        cout << "Negative Cycle Detected\n";
    } else {
        cout << left << setw(10) << "Vertex" << "Distance" << endl;
        for (int i = 1; i <= V; ++i) cout << left << setw(10) << i << (bf_res.second[i] == INF ? -1 : bf_res.second[i]) << "\n";
    }

    cout << "\n" << string(30, '-') << "\nAlgorithm: Floyd-Warshall\n" << string(30, '-') << "\n";
    auto fw_res = g.floydWarshall();
    for (int i = 1; i <= V; ++i) {
        for (int j = 1; j <= V; ++j) {
            if (fw_res[i][j] == INF) cout << setw(5) << "INF";
            else cout << setw(5) << fw_res[i][j];
        }
        cout << "\n";
    }

    cout << "\n" << string(30, '-') << "\nAlgorithm: Johnson\n" << string(30, '-') << "\n";
    auto jh_res = g.johnson();
    if (jh_res.empty()) {
        cout << "Negative Cycle Detected\n";
    } else {
        for (int i = 1; i <= V; ++i) {
            for (int j = 1; j <= V; ++j) {
                if (jh_res[i][j] == INF) cout << setw(5) << "INF";
                else cout << setw(5) << jh_res[i][j];
            }
            cout << "\n";
        }
    }
}

int main() {
    int choice;
    cout << "1. Solve Input from .txt file\n";
    cout << "2. Run Experimental Analysis\n";
    cout << "Enter choice: ";
    cin >> choice;

    if (choice == 1) solveFromFile();
    else if (choice == 2) runExperiments();
    else cout << "Invalid choice.\n";

    return 0;
}