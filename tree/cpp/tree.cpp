#include "tree.h"
#include <algorithm>
#include <cstdio>
using namespace std;
using ll = long long;
int INF = 0x3f3f3f3f;
int n;
vector<ll> costByCnt;
ll initAns;

struct Node {
  int w;
  int par;
  vector<int> c;
  int leafCnt;

  Node() : w(0), par(0), leafCnt(1) {}
};
vector<Node> nodes;

void init(vector<int> P, vector<int> W) {
  n = (int)P.size();
  nodes = vector<Node>(n);
  costByCnt = vector<ll>(n, 0);
  initAns = 0;
  vector<int> order(n);
  for (int i = n - 1; i >= 0; i--) {
    order[i] = i;
    nodes[i].w = W[i];
    nodes[i].par = i;
    if (nodes[i].c.size() == 0)
      initAns += nodes[i].w;
    if (P[i] != -1)
      nodes[P[i]].c.push_back(i);
  }
  sort(order.begin(), order.end(), [&](int i, int j) { return W[i] > W[j]; });
  auto add = [&](int l, int r, ll delta) {
    costByCnt[r] += delta;
    if (--l >= 0)
      costByCnt[l] -= delta;
  };
  for (int i : order) {
    auto &node = nodes[i];
    if (node.c.size() == 0)
      continue;
    int &p = node.par;
    while (nodes[p].par != p)
      p = nodes[p].par;
    auto &parent = nodes[p];
    int leafCnt = parent.leafCnt;
    parent.leafCnt--;
    for (int v : node.c) {
      auto &child = nodes[v];
      parent.leafCnt += child.leafCnt;
      child.par = p;
      add(0, child.leafCnt - 1, -node.w);
    }
    if (leafCnt != parent.leafCnt)
      add(leafCnt, parent.leafCnt - 1, node.w);
  }
  for (int i = costByCnt.size() - 2; i >= 0; i--)
    costByCnt[i] += costByCnt[i + 1];
  for (int i = costByCnt.size() - 2; i >= 0; i--)
    costByCnt[i] += costByCnt[i + 1];
}

ll query(int L, int R) {
  ll cnt = (R - 1) / L + 1;
  ll rest = (L - R % L) % L;
  ll ans = initAns * L;
  ll curCost = cnt < costByCnt.size() ? costByCnt[cnt] : 0;
  ans += curCost * L;
  if (cnt - 1 < costByCnt.size()) {
    ll delta = costByCnt[cnt - 1] - curCost;
    ans += delta * rest;
  }
  return ans;
}
