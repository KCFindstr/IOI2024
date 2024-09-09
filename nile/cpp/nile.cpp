#include "nile.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

using namespace std;
using ll = long long;
using pii = pair<int, int>;
const int INF_INT = 0x3f3f3f3f;
const ll INF = 0x3f3f3f3f3f3f3f3fll;

struct Artifact {
  int w, a, b;

  bool operator<(const Artifact &o) const { return w < o.w; }
};

const int MAXN = 1e5 + 5;

struct Node {
  Node *L, *R;
  int l, r;
  Artifact *al[2], *ar[2];
  ll cost[3][3];
  int updateTrigger;

  Node() : L(nullptr), R(nullptr) { memset(cost, 0, sizeof(cost)); }

  void update(int d) {
    if (d < updateTrigger)
      return;
    L->update(d);
    R->update(d);
    for (int ll = 0; ll < 3; ll++) {
      for (int rr = 0; rr < 3; rr++) {
        cost[ll][rr] = L->cost[ll][0] + R->cost[0][rr];
        if (R->al[0]->w - L->ar[0]->w <= d) {
          cost[ll][rr] = min(cost[ll][rr], L->cost[ll][1] + R->cost[1][rr] +
                                               L->ar[0]->b + R->al[0]->b);
        }
        if (L->l < L->r) {
          if (R->al[0]->w - L->ar[1]->w <= d)
            cost[ll][rr] =
                min(cost[ll][rr], L->cost[ll][2] + R->cost[1][rr] +
                                      L->ar[1]->b + L->ar[0]->a + R->al[0]->b);
        } else if (ll == 2) {
          cost[ll][rr] = min(cost[ll][rr], R->cost[1][rr]);
        }
        if (R->l < R->r) {
          if (R->al[1]->w - L->ar[0]->w <= d)
            cost[ll][rr] =
                min(cost[ll][rr], L->cost[ll][1] + R->cost[2][rr] +
                                      L->ar[0]->b + R->al[0]->a + R->al[1]->b);
        } else if (rr == 2) {
          cost[ll][rr] = min(cost[ll][rr], L->cost[ll][1]);
        }
      }
    }
    this->updateTrigger = min(L->updateTrigger, R->updateTrigger);
    for (int lr = 0; lr < 2; lr++) {
      for (int rl = 0; rl < 2 - lr; rl++) {
        int diff = R->al[rl]->w - L->ar[lr]->w;
        if (diff <= d)
          continue;
        this->updateTrigger = min(this->updateTrigger, diff);
      }
    }
  }
} nodes[MAXN << 2];

struct SegmentTree {
  vector<Artifact> artifacts;
  int nodeCnt;
  Node *root;
  SegmentTree(const vector<Artifact> &A) : artifacts(A), nodeCnt(0) {
    root = build(0, artifacts.size() - 1);
  }

  Node *build(int l, int r) {
    Node *ret = &nodes[nodeCnt++];
    ret->l = l;
    ret->r = r;
    ret->al[0] = &artifacts[l];
    ret->ar[0] = &artifacts[r];
    if (l == r) {
      ret->cost[0][0] = artifacts[l].a;
      ret->cost[1][0] = 0;
      ret->cost[0][1] = 0;
      ret->cost[1][1] = INF;
      ret->cost[2][0] = INF;
      ret->cost[0][2] = INF;
      ret->cost[2][1] = INF;
      ret->cost[1][2] = INF;
      ret->cost[2][2] = INF;
      ret->updateTrigger = INF_INT;
      ret->al[1] = &artifacts[l];
      ret->ar[1] = &artifacts[r];
      return ret;
    }
    int m = (l + r) >> 1;
    ret->L = build(l, m);
    ret->R = build(m + 1, r);
    ret->al[1] = &artifacts[l + 1];
    ret->ar[1] = &artifacts[r - 1];
    ret->update(0);
    return ret;
  }
};

vector<long long> calculate_costs(vector<int> W, vector<int> A, vector<int> B,
                                  vector<int> E) {
  vector<Artifact> artifacts(W.size());
  for (int i = 0; i < W.size(); i++)
    artifacts[i] = {W[i], A[i], B[i]};
  sort(artifacts.begin(), artifacts.end());
  vector<pii> questions(E.size());
  for (int i = 0; i < E.size(); i++)
    questions[i] = {E[i], i};
  sort(questions.begin(), questions.end());
  SegmentTree tree(artifacts);
  vector<ll> R(E.size());
  for (auto [d, i] : questions) {
    tree.root->update(d);
    R[i] = tree.root->cost[0][0];
  }
  return R;
}
