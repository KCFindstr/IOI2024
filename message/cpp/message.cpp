#include "message.h"

using namespace std;
const int MAX_BIT = 31;

void send_message(vector<bool> M, vector<bool> C) {
  int nxt[MAX_BIT] = {0};
  int prev = -1;
  for (int i = 0; prev < MAX_BIT; i++) {
    if (C[i % MAX_BIT])
      continue;
    if (prev >= 0)
      nxt[prev] = i - prev - 1;
    prev = i;
  }
  M.push_back(true);
  int idx = 0;
  for (int j = 0; j < 16 || idx < M.size(); j++) {
    vector<bool> A(MAX_BIT, false);
    for (int i = 0; i < MAX_BIT; i++) {
      if (C[i])
        continue;
      if (j > nxt[i])
        A[i] = idx >= M.size() ? false : M[idx++];
      else if (j == nxt[i])
        A[i] = true;
      else
        A[i] = false;
    }
    send_packet(A);
  }
}

vector<bool> receive_message(vector<vector<bool>> R) {
  vector<bool> M;
  bool C[MAX_BIT] = {0};
  int nxt[MAX_BIT] = {0};
  for (int i = 0; i < MAX_BIT; i++)
    for (int &j = nxt[i]; j < R.size() && !R[j][i]; j++)
      ;

  for (int i = 0; i < MAX_BIT; i++) {
    int cur = i;
    C[i] = true;
    for (int j = 0; j < 16; j++) {
      cur = (cur + nxt[cur] + 1) % MAX_BIT;
      if (cur == i) {
        C[i] = j != 15;
        break;
      }
    }
  }
  for (int i = 0; i < R.size(); i++) {
    for (int j = 0; j < MAX_BIT; j++) {
      if (C[j] || i <= nxt[j])
        continue;
      M.push_back(R[i][j]);
    }
  }
  while (!M.back())
    M.pop_back();
  M.pop_back();
  return M;
}
