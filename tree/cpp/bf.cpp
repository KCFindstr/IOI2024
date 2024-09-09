#include "tree.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <set>
using namespace std;
using ll = long long;
using pll = pair<ll, ll>;
using pii = pair<int, int>;
int INF = 0x3f3f3f3f;
ll INFLL = 0x3f3f3f3f3f3f3f3fll;
int n;
ll initAns;

struct Node {
  int w;
  int p;
  vector<int> c;
  int coef;
  int totCoef;
};
vector<Node> nodes;
vector<ll> costByCnt;

void addSubtree(int u, int delta) {
  auto &node = nodes[u];
  node.w += delta;
  for (auto v : node.c)
    addSubtree(v, delta);
}

struct MinIdxData {
  int idx;
  int saveIdx;
  int cost;
};

int findMinIdx(int u) {
  const auto &node = nodes[u];
  if (node.totCoef <= 1)
    return -1;
  int ret = u;
  for (int v : node.c) {
    int child = findMinIdx(v);
    if (child != -1 && nodes[child].w < nodes[ret].w)
      ret = child;
  }
  return ret;
}

int decrease(int u, int p) {
  p = nodes[p].p;
  int maxSaveIdx = -1;
  while (p != -1) {
    auto &node = nodes[p];
    assert(node.totCoef > 0);
    if (node.totCoef == 1) {
      assert(node.coef < 0);
      if (maxSaveIdx == -1 || nodes[maxSaveIdx].w < node.w)
        maxSaveIdx = p;
      assert(maxSaveIdx != -1);
      break;
    }
    if (node.coef < 0 && (maxSaveIdx == -1 || nodes[maxSaveIdx].w < node.w))
      maxSaveIdx = p;
    p = node.p;
  }
  nodes[u].coef--;
  while (u != maxSaveIdx) {
    auto &node = nodes[u];
    node.totCoef--;
    u = node.p;
  }
  if (maxSaveIdx != -1) {
    nodes[maxSaveIdx].coef++;
    return nodes[maxSaveIdx].w;
  }
  return 0;
}

void init(vector<int> P, vector<int> W) {
  n = (int)P.size();
  nodes = vector<Node>(n);
  costByCnt = vector<ll>(n + 1, 0);
  initAns = 0;
  for (int i = n - 1; i >= 0; i--) {
    nodes[i].w = W[i];
    nodes[i].p = P[i];
    if (nodes[i].c.size() == 0) {
      nodes[i].totCoef = nodes[i].coef = 1;
      initAns += nodes[i].w;
    }
    if (P[i] != -1) {
      nodes[P[i]].c.push_back(i);
      nodes[P[i]].totCoef += nodes[i].totCoef;
    }
  }
  // Sanity check
  ll expectedAns1 = 0;
  for (int i = 0; i < n; i++) {
    if (nodes[i].c.size() > 0)
      expectedAns1 += (ll)W[i] * (nodes[i].c.size() - 1);
  }
  for (int i = n - 1; i >= 1; i--) {
    ll &cost = costByCnt[i];
    cost += costByCnt[i + 1];
    for (int j = 0; j < n; j++) {
      if (nodes[j].totCoef <= i)
        continue;
      int idx = findMinIdx(j);
      assert(idx != -1 && nodes[idx].totCoef > 1 && nodes[idx].coef <= 0);
      cost += nodes[idx].w;
      cost -= decrease(idx, j);
    }
  }
  for (int i = 0; i < n; i++) {
    assert(nodes[i].totCoef == 1);
  }
  assert(expectedAns1 == costByCnt[1]);
}

ll query(int L, int R) {
  ll cnt = R / L;
  ll rest = L - R % L;
  ll ans = initAns * L;
  if (cnt >= n)
    return ans;
  ans += costByCnt[cnt + 1] * L;
  ll delta = costByCnt[cnt] - costByCnt[cnt + 1];
  assert(delta >= 0);
  ans += delta * rest;
  return ans;
}
