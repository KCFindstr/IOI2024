#include "mosaic.h"

#include <cstdio>
#include <vector>

using namespace std;
using ll = long long;
const int BRUTE_FORCE = 3;

struct Rect {
  int t, b, l, r;
  int width() const { return r - l + 1; }
  int height() const { return b - t + 1; }
};

struct FirstRowCol {
  vector<int> rows[BRUTE_FORCE], cols[BRUTE_FORCE];
  vector<int> totRow[BRUTE_FORCE], totCol[BRUTE_FORCE];
  int cnt;

  FirstRowCol(const vector<int> &X, const vector<int> &Y) {
    rows[0] = X;
    cols[0] = Y;
    cnt = min(BRUTE_FORCE, (int)X.size());
    for (int i = 1; i < cnt; i++) {
      rows[i].resize(X.size());
      for (int j = 0; j < X.size(); j++) {
        if (j < i && j < cnt)
          rows[i][j] = cols[j][i];
        else
          rows[i][j] = !(rows[i - 1][j] || rows[i][j - 1]);
      }
      cols[i].resize(Y.size());
      for (int j = 0; j < Y.size(); j++) {
        if (j < i && j < cnt)
          cols[i][j] = rows[j][i];
        else
          cols[i][j] = !(cols[i - 1][j] || cols[i][j - 1]);
      }
    }
    for (int i = 0; i < cnt; i++) {
      totRow[i].push_back(0);
      totCol[i].push_back(0);
      for (int j = 0; j < X.size(); j++) {
        totRow[i].push_back(totRow[i].back() + rows[i][j]);
        totCol[i].push_back(totCol[i].back() + cols[i][j]);
      }
    }
  }

  int calc(Rect q) {
    int ret = 0;
    for (int i = q.t - 1; i < q.b && i < cnt; i++)
      ret += totRow[i][q.r] - totRow[i][q.l - 1];
    for (int i = q.l - 1; i < q.r && i < cnt; i++)
      ret += totCol[i][q.b] - totCol[i][q.t - 1];
    for (int i = q.t - 1; i < q.b && i < cnt; i++)
      for (int j = q.l - 1; j < q.r && j < cnt; j++)
        ret -= rows[i][j];
    return ret;
  }
};

struct RestRowCol {
  struct PrefixSum {
    ll sum, sumXIdx;

    PrefixSum append(int idx, int val) {
      return {sum + val, sumXIdx + (ll)val * idx};
    }

    PrefixSum operator-(const PrefixSum &rhs) {
      return {sum - rhs.sum, sumXIdx - rhs.sumXIdx};
    }
  };
  int offset;
  int idx;
  vector<PrefixSum> sums;

  // Index: y - x
  PrefixSum &operator[](int idx) {
    idx = max(0, min((int)sums.size() - 1, idx + offset));
    return sums[idx];
  }

  RestRowCol(const vector<int> &X, const vector<int> &Y, int idx)
      : offset(Y.size() - idx), idx(idx) {
    sums.push_back({0, 0});

    for (int i = Y.size() - 1; i >= idx; i--) {
      sums.push_back(sums.back().append(idx - i, Y[i]));
    }
    for (int i = idx + 1; i < X.size(); i++) {
      sums.push_back(sums.back().append(i - idx, X[i]));
    }
  }

  ll calcTriangle(Rect q, bool bottomLeft) {
    int y = bottomLeft ? q.l : q.r;
    int upper = y - q.t;
    int lower = y - q.b - 1;
    PrefixSum sum = (*this)[upper] - (*this)[lower];
    if (bottomLeft) {
      return sum.sumXIdx - sum.sum * lower;
    } else {
      return sum.sum * (upper + 1) - sum.sumXIdx;
    }
  }

  ll calcStrip(int start, int end, int width) {
    PrefixSum sum = (*this)[end] - (*this)[start - 1];
    return sum.sum * width;
  }

  ll calc(Rect q) {
    q.t = max(q.t, idx + 2);
    q.l = max(q.l, idx + 2);
    int size = min(q.width(), q.height());
    if (size <= 0)
      return 0;
    ll ret = 0;
    Rect lb = {q.b - size + 1, q.b, q.l, q.l + size - 1};
    Rect rt = {q.t, q.t + size - 1, q.r - size + 1, q.r};
    ret += calcTriangle(lb, true);
    ret += calcTriangle(rt, false);
    ret += calcStrip(lb.l - lb.t + 1, rt.r - rt.b - 1, size);
    return ret;
  }
};

vector<long long> mosaic(vector<int> X, vector<int> Y, vector<int> T,
                         vector<int> B, vector<int> L, vector<int> R) {
  FirstRowCol frc(X, Y);
  int idx = frc.cnt - 1;
  RestRowCol rrc(frc.rows[idx], frc.cols[idx], idx);

  vector<long long> C;
  for (int i = 0; i < T.size(); i++) {
    Rect r = {T[i] + 1, B[i] + 1, L[i] + 1, R[i] + 1};
    ll ans = frc.calc(r) + rrc.calc(r);
    C.push_back(ans);
  }
  return C;
}
