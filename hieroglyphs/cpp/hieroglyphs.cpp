#include "hieroglyphs.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <stack>
#include <vector>
using namespace std;
using pii = pair<int, int>;
const int ALPHABET = 200010;

struct Interval {
  int l, r;
};

struct SequenceData {
  vector<int> data;
  vector<int> nxt;
  vector<int> nxtCtrl;
  vector<int> matchedPos;
  vector<int> matchedIdx;
  vector<int> firstUnmatched;
  vector<int> lastUnmatched;

  SequenceData(const vector<int> &input)
      : data(input), nxt(input.size(), input.size()),
        nxtCtrl(input.size(), input.size()), matchedIdx(input.size(), -1),
        firstUnmatched(ALPHABET, -1), lastUnmatched(ALPHABET, -1) {
    vector<int> prev(ALPHABET, -1);
    for (int i = 0; i < input.size(); i++) {
      if (prev[input[i]] != -1)
        nxt[prev[input[i]]] = i;
      prev[input[i]] = i;
    }
  }

  void finalizeMatch() {
    for (int i = 0; i < matchedPos.size(); i++)
      matchedIdx[matchedPos[i]] = i;
    for (int i = 0; i < data.size(); i++) {
      if (matchedIdx[i] == -1) {
        if (firstUnmatched[data[i]] == -1)
          firstUnmatched[data[i]] = i;
        lastUnmatched[data[i]] = i;
      }
    }
  }

  vector<int> matchSeq() const {
    vector<int> ret;
    for (int i : matchedPos)
      ret.push_back(data[i]);
    return ret;
  }

  int &operator[](int idx) { return data[idx]; }
  const int &operator[](int idx) const { return data[idx]; }
  int size() const { return data.size(); }
};

struct SequenceIter {
  const SequenceData &data;
  int pos;
  vector<int> cnt;
  vector<int> nxt;

  SequenceIter(const SequenceData &data)
      : data(data), pos(0), cnt(ALPHABET), nxt(ALPHABET) {
    for (int i = data.size() - 1; i >= 0; i--) {
      nxt[data[i]] = i;
      cnt[data[i]]++;
    }
  }

  void advance() {
    if (pos < data.size()) {
      cnt[data[pos]]--;
      nxt[data[pos]] = data.nxt[pos];
      pos++;
    }
  }

  void advance(int idx) {
    assert(pos <= idx);
    while (pos < idx)
      advance();
  }

  void advanceCtrl() {
    if (!finished())
      advance(data.nxtCtrl[pos]);
  }

  bool finished() const { return pos >= data.size(); }

  int val() const { return data[pos]; }
};

void computeControl(SequenceData &A, SequenceData &B, const SequenceIter &iA,
                    const SequenceIter &iB) {
  int nxt = A.size();
  for (int i = A.size() - 1; i >= 0; i--) {
    if (iA.cnt[A[i]] <= iB.cnt[A[i]])
      nxt = i;
    A.nxtCtrl[i] = nxt;
  }
  nxt = B.size();
  for (int i = B.size() - 1; i >= 0; i--) {
    if (iB.cnt[B[i]] < iA.cnt[B[i]])
      nxt = i;
    B.nxtCtrl[i] = nxt;
  }
}

bool construct(SequenceData &A, SequenceData &B) {
  SequenceIter iA(A), iB(B);
  SequenceIter cA(iA), cB(iB);
  computeControl(A, B, iA, iB);
  int expectedCount = 0;
  for (int i = 0; i < ALPHABET; i++)
    expectedCount += min(iA.cnt[i], iB.cnt[i]);
  cA.advanceCtrl();
  cB.advanceCtrl();
  while (!cA.finished() || !cB.finished()) {
    bool advanceA;
    if (cA.finished()) {
      advanceA = false;
    } else if (cB.finished()) {
      advanceA = true;
    } else {
      int nxtBinA = iA.nxt[cB.val()];
      int nxtAinB = iB.nxt[cA.val()];
      if (nxtBinA > cA.pos) {
        advanceA = true;
      } else if (nxtAinB > cB.pos) {
        advanceA = false;
      } else {
        iA.advance(nxtBinA);
        int cntAinA = iA.cnt[cA.val()];
        int cntAinB = cB.cnt[cA.val()];
        advanceA = cntAinA > cntAinB;
      }
    }

    if (advanceA) {
      A.matchedPos.push_back(cA.pos);
      iA.advance(cA.pos + 1);
      iB.advance(iB.nxt[cA.val()]);
      if (iB.finished())
        return false;
      B.matchedPos.push_back(iB.pos);
      iB.advance();
      cA.advance();
      cA.advanceCtrl();
    } else {
      B.matchedPos.push_back(cB.pos);
      iB.advance(cB.pos + 1);
      iA.advance(iA.nxt[cB.val()]);
      if (iA.finished())
        return false;
      A.matchedPos.push_back(iA.pos);
      iA.advance();
      cB.advance();
      cB.advanceCtrl();
    }
  }
  if (A.matchedPos.size() < expectedCount)
    return false;
  A.finalizeMatch();
  B.finalizeMatch();
  return true;
}

bool check(const SequenceData &A, const SequenceData &B) {
  stack<Interval> s;
  vector<vector<int>> query(A.size());
  vector<vector<int>> cover(B.size());
  vector<bool> covered(ALPHABET);
  for (int i = 0; i < A.size(); i++) {
    int first = A.firstUnmatched[A[i]];
    if (first == -1 || !query[first].empty())
      continue;
    query[first].push_back(A[i]);
  }
  for (int i = A.size() - 1; i >= 0; i--) {
    int val = A[i];
    while (!s.empty() && s.top().l >= i)
      s.pop();
    if (A.matchedIdx[i] != -1) {
      if (!s.empty()) {
        query[s.top().r].push_back(val);
      }
      int first = A.firstUnmatched[val];
      if (first != -1) {
        while (!s.empty() && s.top().l >= first)
          s.pop();
        s.push({first, i});
      }
    }
  }
  while (!s.empty())
    s.pop();
  for (int i = 0; i < B.size(); i++) {
    int val = B[i];
    while (!s.empty() && s.top().r <= i)
      s.pop();
    if (B.matchedIdx[i] != -1) {
      if (!s.empty()) {
        cover[s.top().l].push_back(val);
      }
      int last = B.lastUnmatched[val];
      if (last != -1) {
        while (!s.empty() && s.top().r <= last)
          s.pop();
        s.push({i, last});
      }
    }
  }
  for (int i = A.size() - 1; i >= 0; i--) {
    for (int j : query[i]) {
      if (covered[j]) {
        return false;
      }
    }
    if (A.matchedIdx[i] != -1) {
      for (int j : cover[B.matchedPos[A.matchedIdx[i]]])
        covered[j] = true;
    }
  }
  return true;
}

vector<int> ucs(vector<int> A, vector<int> B) {
  auto dA = SequenceData(A);
  auto dB = SequenceData(B);
  if (!construct(dA, dB) || !check(dA, dB) || !check(dB, dA))
    return {-1};
  return dA.matchSeq();
}
