#include "sphinx.h"
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <queue>
#include <set>

using namespace std;
const int KEEP = -1;

struct Graph {
  int n;
  vector<vector<int>> adj;
  vector<int> component;
  vector<vector<int>> members;

  vector<int> layers[2];

  Graph(int n) : n(n), adj(n), component(n, -1), members(n) {}

  void buildLayers() {
    vector<set<int>> cAdj(n);
    for (int u = 0; u < n; u++) {
      for (int v : adj[u]) {
        int cu = component[u], cv = component[v];
        if (cu == cv)
          continue;
        cAdj[cu].insert(cv);
        cAdj[cv].insert(cu);
      }
    }
    vector<int> visited(n, -1);
    queue<int> q;
    q.push(component[0]);
    visited[q.front()] = 0;
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      layers[visited[u]].push_back(u);
      for (int v : cAdj[u]) {
        if (visited[v] >= 0)
          continue;
        visited[v] = visited[u] ^ 1;
        q.push(v);
      }
    }
  }

  vector<int> getComponents() const {
    vector<int> ret;
    for (int i = 0; i < n; i++)
      if (!members[i].empty())
        ret.push_back(i);
    return ret;
  }

  void addEdge(int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  }

  void setComponent(int u, int c) {
    component[u] = c;
    members[c].push_back(u);
  }

  void merge(int u, int v) {
    u = component[u];
    v = component[v];
    for (auto vn : members[v]) {
      component[vn] = u;
      members[u].push_back(vn);
    }
    members[v].clear();
  }

  int checkComponents(vector<int> query) {
    vector<bool> visited(n, false);
    int ret = 0;
    for (int i = 0; i < n; i++) {
      if (visited[i])
        continue;
      ret++;
      queue<int> q;
      q.push(i);
      visited[i] = true;
      while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : adj[u]) {
          if (visited[v] || query[v] != query[u])
            continue;
          visited[v] = true;
          q.push(v);
        }
      }
    }
    return ret;
  }
};

int merge_component(Graph &g, int u, vector<int> t, bool mustHave = false) {
  if (t.size() == 0)
    return 0;
  int expectCount = -1;
  if (!mustHave) {
    vector<int> query(g.n, g.n), graph(g.n, g.n);
    query[u] = graph[u] = KEEP;
    for (auto v : t)
      for (auto u : g.members[v])
        query[u] = KEEP, graph[u] = v;
    int expected = g.checkComponents(graph);
    int actual = perform_experiment(query);
    expectCount = expected - actual;
    if (expectCount == 0)
      return 0;
  }
  if (t.size() == 1) {
    if (g.component[u] == -1)
      g.setComponent(u, t[0]);
    else
      g.merge(u, t[0]);
    return 1;
  }
  int m = t.size() >> 1;
  int lAns = merge_component(g, u, vector<int>(t.begin(), t.begin() + m));
  if (expectCount == lAns)
    return lAns;
  int rAns = merge_component(g, u, vector<int>(t.begin() + m, t.end()), !lAns);
  return lAns + rAns;
}

void determine_components(Graph &g) {
  for (int i = 0; i < g.n; i++) {
    if (g.component[i] != -1)
      continue;
    vector<int> t = g.getComponents();
    if (merge_component(g, i, t))
      continue;
    g.setComponent(i, i);
  }
}

bool determine_color(Graph &g, vector<int> &colors, int c, vector<int> t,
                     bool mustHave = false) {
  int expected = -1, actual = -1;
  set<int> toCheck(t.begin(), t.end());
  vector<int> query(g.n, c), graph(g.n, g.n);
  for (int i = 0; i < g.n; i++)
    if (toCheck.count(g.component[i]))
      query[i] = KEEP, graph[i] = g.component[i];
  if (!mustHave) {
    expected = g.checkComponents(graph);
    actual = perform_experiment(query);
    if (expected == actual)
      return false;
  }
  if (t.size() == 1) {
    for (auto v : g.members[t[0]])
      colors[v] = c;
    return true;
  }
  int m = t.size() >> 1;
  bool lAns =
      determine_color(g, colors, c, vector<int>(t.begin(), t.begin() + m));
  if (lAns) {
    for (int i = 0; i < colors.size(); i++)
      if (colors[i] == c)
        graph[i] = g.n;
    if (g.checkComponents(graph) == actual)
      return lAns;
  }
  bool rAns =
      determine_color(g, colors, c, vector<int>(t.begin() + m, t.end()), !lAns);
  return lAns || rAns;
}

vector<int> determine_color(Graph &g) {
  if (g.getComponents().size() == 1) {
    for (int i = 0; i < g.n; i++) {
      vector<int> query(g.n, i);
      query[0] = KEEP;
      if (perform_experiment(query) == 1)
        return vector<int>(g.n, i);
    }
  }
  vector<int> colors(g.n, -1);
  for (int i = 1; i < g.n; i++) {
    for (auto &l : g.layers) {
      if (determine_color(g, colors, i, l)) {
        for (auto it = l.begin(); it != l.end();)
          if (colors[*it] != -1)
            it = l.erase(it);
          else
            it++;
      }
    }
  }
  for (int i = 0; i < g.n; i++)
    if (colors[i] == -1)
      colors[i] = 0;
  return colors;
}

vector<int> find_colours(int N, vector<int> X, vector<int> Y) {
  Graph g(N);
  for (int i = 0; i < X.size(); i++)
    g.addEdge(X[i], Y[i]);
  determine_components(g);
  g.buildLayers();
  return determine_color(g);
}
