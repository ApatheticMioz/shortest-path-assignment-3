/**
 * benchmark.cpp
 * Statistical benchmark harness for shortest-path algorithms.
 * Runs 500 iterations per graph type with mean, median, std dev, and 95% CI.
 */

#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <limits>
#include <chrono>
#include <random>
#include <iomanip>
#include <fstream>
#include <string>
#include <cmath>
#include <numeric>
#include <queue> 

using namespace std;

const long long INF = 1e18;

// --- Statistics Helper ---
struct StatResult {
    double mean;
    double median;
    double stdDev;
    double ci95;
};

StatResult calculateStats(vector<double>& data) {
    if (data.empty()) return {0,0,0,0};
    double sum = accumulate(data.begin(), data.end(), 0.0);
    double mean = sum / data.size();
    double sq_sum = inner_product(data.begin(), data.end(), data.begin(), 0.0);
    double stdDev = sqrt(max(0.0, sq_sum / data.size() - mean * mean));
    sort(data.begin(), data.end());
    double median = (data.size()%2==0) ? (data[data.size()/2-1]+data[data.size()/2])/2.0 : data[data.size()/2];
    double ci = 1.96 * (stdDev / sqrt(data.size()));
    return {mean, median, stdDev, ci};
}

class Graph {
    int V;
    int maxV;
    // CHANGED: Use vector of vectors instead of raw pointer array (Safe Memory)
    vector<vector<pair<int, long long>>> adj; 
    // CHANGED: Use flattened vector instead of raw pointer (Safe Memory)
    vector<long long> matrix;                 
    bool isDirected;

public:
    long long relaxationCount; 

    Graph(int maxVertices) {
        maxV = maxVertices + 5; 
        // Pre-allocate memory ONCE
        adj.resize(maxV); 
        matrix.resize(maxV * maxV, INF);
        V = 1;
        relaxationCount = 0;
    }

    // No Destructor needed! std::vector cleans itself up safely.

    void reset(int vertices, bool directed) {
        V = vertices;
        isDirected = directed;
        relaxationCount = 0;

        // Clear adjacency lists (capacity is kept, so this is fast)
        for(int i = 0; i <= V + 1; ++i) {
            adj[i].clear();
        }
        
        // Fast Matrix Reset
        // Only reset the sub-grid we are about to use to INF
        // We iterate 0..V (inclusive)
        for(int i = 0; i <= V; ++i) {
            int rowStart = i * maxV;
            for(int j = 0; j <= V; ++j) {
                matrix[rowStart + j] = (i == j) ? 0 : INF;
            }
        }
    }

    void addEdge(int u, int v, long long w) {
        // Safety check to prevent segfaults from bad generation
        if (u >= maxV || v >= maxV || u < 0 || v < 0) return;

        adj[u].push_back({v, w});
        
        int idx = u * maxV + v;
        if(w < matrix[idx]) matrix[idx] = w;

        if (!isDirected) {
            adj[v].push_back({u, w});
            int idxRev = v * maxV + u;
            if(w < matrix[idxRev]) matrix[idxRev] = w;
        }
    }

    double getMemoryUsage(string algoType, int E) {
        double bytes = 0;
        if (algoType == "Floyd-Warshall") bytes = V * V * sizeof(long long); 
        else {
            bytes = V * sizeof(vector<pair<int,long long>>) + E * sizeof(pair<int,long long>) + V * sizeof(long long);
            if(algoType == "Johnson") bytes += V * sizeof(long long); 
        }
        return bytes / 1024.0; 
    }

    // --- ALGORITHMS ---

    void dijkstra(int src) {
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
                relaxationCount++;
                if (dist[u] != INF && dist[u] + edge.second < dist[edge.first]) {
                    dist[edge.first] = dist[u] + edge.second;
                    pq.push({dist[edge.first], edge.first});
                }
            }
        }
    }

    void bellmanFord(int src) {
        relaxationCount = 0;
        vector<long long> dist(V + 1, INF);
        dist[src] = 0;
        bool changed = false;
        for (int i = 1; i <= V - 1; ++i) {
            changed = false;
            for (int u = 1; u <= V; ++u) {
                if (dist[u] == INF) continue;
                for (auto& edge : adj[u]) {
                    relaxationCount++;
                    if (dist[u] + edge.second < dist[edge.first]) {
                        dist[edge.first] = dist[u] + edge.second;
                        changed = true;
                    }
                }
            }
            if (!changed) break;
        }
    }

    void floydWarshall() {
        relaxationCount = 0;
        // Use a local copy to avoid modifying the graph state for other algos
        int localW = V + 1;
        vector<long long> d(localW * localW);
        
        // Copy submatrix
        for(int i=1; i<=V; ++i)
            for(int j=1; j<=V; ++j)
                d[i*localW + j] = matrix[i*maxV + j];

        for (int k = 1; k <= V; ++k) {
            for (int i = 1; i <= V; ++i) {
                if(d[i*localW + k] == INF) continue;
                for (int j = 1; j <= V; ++j) {
                    if(d[k*localW + j] != INF) {
                        relaxationCount++;
                        if (d[i*localW + k] + d[k*localW + j] < d[i*localW + j]) {
                            d[i*localW + j] = d[i*localW + k] + d[k*localW + j];
                        }
                    }
                }
            }
        }
    }

    void johnson() {
        relaxationCount = 0;
        // 1. Add virtual source V+1
        int s = V + 1;
        // Check bounds
        if (s < adj.size()) adj[s].push_back({1, 0}); // Add just one edge to kickstart loop below? 
        // Actually Johnson connects s to ALL vertices
        for (int i = 1; i <= V; ++i) {
            adj[s].push_back({i, 0});
        }
        
        // 2. BF for potentials
        vector<long long> h(V + 2, INF);
        h[s] = 0;
        
        bool changed = false;
        // BF on V+1 nodes
        for (int i = 1; i <= V; ++i) { 
            changed = false;
            for (int u = 1; u <= s; ++u) {
                if (h[u] == INF) continue;
                for (auto& edge : adj[u]) {
                    relaxationCount++;
                    if (h[u] + edge.second < h[edge.first]) {
                        h[edge.first] = h[u] + edge.second;
                        changed = true;
                    }
                }
            }
            if (!changed) break;
        }

        // 3. Negative Cycle Check (Fix for Infinite Loop)
        for (int u = 1; u <= s; ++u) {
            if (h[u] == INF) continue;
            for (auto& edge : adj[u]) {
                 if (h[u] + edge.second < h[edge.first]) return; // CYCLE DETECTED, EXIT
            }
        }

        // 4. Reweight & Run Dijkstra
        // Modify weights in-place (reset() cleans this up later)
        for (int u = 1; u <= V; ++u) {
            for (auto& edge : adj[u]) {
                edge.second = edge.second + h[u] - h[edge.first];
            }
        }
        
        for (int u = 1; u <= V; ++u) {
            long long currentRelax = relaxationCount;
            dijkstra(u);
            relaxationCount += currentRelax; 
        }
    }
};

void generateConnectedGraph(int V, int E, bool allowNegative, vector<tuple<int, int, int>>& outEdges) {
    outEdges.clear();
    outEdges.reserve(E);
    static random_device rd; static mt19937 gen(rd());
    uniform_int_distribution<> weights(allowNegative ? -5 : 1, 20);
    
    vector<int> nodes(V); 
    for(int i=0; i<V; ++i) nodes[i] = i + 1;
    shuffle(nodes.begin(), nodes.end(), gen);
    
    // Spanning Tree (Connectivity)
    for (int i = 1; i < V; ++i) {
        uniform_int_distribution<> connectedDist(0, i - 1);
        outEdges.emplace_back(nodes[i], nodes[connectedDist(gen)], weights(gen));
    }
    
    // Remaining Edges
    uniform_int_distribution<> nodeDist(1, V);
    int remaining = E - (V - 1);
    while (remaining > 0) {
        int u = nodeDist(gen), v = nodeDist(gen);
        if (u != v) { outEdges.emplace_back(u, v, weights(gen)); remaining--; }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ofstream report("statistics_report.txt");
    cout << "Running Benchmark (500 Iterations)..." << endl;

    struct Config { string type; int V_min, V_max; double dens; bool neg; };
    vector<Config> configs = {
        {"Sparse", 10, 50, 1.2, false}, 
        {"Dense", 100, 200, 0.8, false},
        {"Mixed", 10, 50, 2.0, true}
    };

    random_device rd; mt19937 gen(rd());
    const int ITERATIONS = 500;

    // Safe Graph Allocation (Max V = 220)
    Graph g(220);

    for (auto& cfg : configs) {
        cout << "Processing " << cfg.type << "..." << endl;
        report << "\n================ " << cfg.type << " (" << ITERATIONS << " runs) ================\n";
        
        vector<double> t_dijk, t_bf, t_fw, t_john;
        vector<double> r_dijk, r_bf, r_fw, r_john;
        double mem_dijk=0, mem_bf=0, mem_fw=0, mem_john=0;

        // Reserve to prevent reallocation during timing
        t_dijk.reserve(ITERATIONS); r_dijk.reserve(ITERATIONS);
        t_bf.reserve(ITERATIONS); r_bf.reserve(ITERATIONS);
        t_fw.reserve(ITERATIONS); r_fw.reserve(ITERATIONS);
        t_john.reserve(ITERATIONS); r_john.reserve(ITERATIONS);

        for(int i=0; i<ITERATIONS; ++i) {
            if(i % 50 == 0) cout << "\r " << (i*100/ITERATIONS) << "%" << flush;

            int V = uniform_int_distribution<>(cfg.V_min, cfg.V_max)(gen);
            int E = (cfg.type == "Dense") ? V*(V-1)*cfg.dens : V*cfg.dens;
            
            vector<tuple<int, int, int>> edges;
            generateConnectedGraph(V, E, cfg.neg, edges);
            
            g.reset(V, true);
            for(auto [u, v, w] : edges) g.addEdge(u, v, w);

            if(i==0) {
                mem_dijk = g.getMemoryUsage("Dijkstra", edges.size());
                mem_bf = g.getMemoryUsage("Bellman-Ford", edges.size());
                mem_fw = g.getMemoryUsage("Floyd-Warshall", edges.size());
                mem_john = g.getMemoryUsage("Johnson", edges.size());
            }

            if(!cfg.neg) {
                auto start = chrono::high_resolution_clock::now();
                g.dijkstra(1);
                t_dijk.push_back(chrono::duration<double, milli>(chrono::high_resolution_clock::now() - start).count());
                r_dijk.push_back(g.relaxationCount);
            }

            auto start = chrono::high_resolution_clock::now();
            g.bellmanFord(1);
            t_bf.push_back(chrono::duration<double, milli>(chrono::high_resolution_clock::now() - start).count());
            r_bf.push_back(g.relaxationCount);

            start = chrono::high_resolution_clock::now();
            g.floydWarshall();
            t_fw.push_back(chrono::duration<double, milli>(chrono::high_resolution_clock::now() - start).count());
            r_fw.push_back(g.relaxationCount);

            start = chrono::high_resolution_clock::now();
            g.johnson();
            t_john.push_back(chrono::duration<double, milli>(chrono::high_resolution_clock::now() - start).count());
            r_john.push_back(g.relaxationCount);
        }
        cout << "\r 100% Done." << endl;

        auto printStat = [&](string name, vector<double>& t, vector<double>& r, double m) {
            if(t.empty()) { report << name << ": Skipped\n"; return; }
            StatResult st = calculateStats(t);
            StatResult sr = calculateStats(r);
            report << left << setw(15) << name 
                   << " | T(ms): " << st.mean << " (+/-" << st.ci95 << ")"
                   << " | Relax: " << sr.mean 
                   << " | Mem: " << m << " KB\n";
        };

        printStat("Dijkstra", t_dijk, r_dijk, mem_dijk);
        printStat("Bellman-Ford", t_bf, r_bf, mem_bf);
        printStat("Floyd-Warshall", t_fw, r_fw, mem_fw);
        printStat("Johnson", t_john, r_john, mem_john);
    }
    cout << "Results saved to statistics_report.txt" << endl;
    return 0;
}