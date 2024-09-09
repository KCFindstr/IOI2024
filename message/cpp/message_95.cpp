#include "message.h"
#include <algorithm>
#include <cstdio>
#include <random>

using namespace std;
using ll = long long;
using uint = unsigned int;
const int MAX_BIT = 31;
const int SEED = 0x1a2b3c4d;
uint MAGIC_NUMBERS[MAX_BIT];

void init() {
  static bool initialized = false;
  if (initialized)
    return;
  initialized = true;

  mt19937 rng(SEED);
  for (int i = 0; i < MAX_BIT; i++)
    MAGIC_NUMBERS[i] = rng();
}

vector<bool> uint_to_vector(uint x, int len) {
  vector<bool> v;
  for (int i = 0; i < len; i++)
    v.push_back((x >> i) & 1);
  return v;
}

uint vector_to_uint(const vector<bool> &v) {
  uint x = 0;
  for (uint i = 0; i < v.size(); i++)
    x |= v[i] << i;
  return x;
}

struct DataSequence {
  vector<bool> data;
  uint idx;
  DataSequence(vector<bool> data = vector<bool>()) : data(data), idx(0) {}
  bool finished() const { return idx >= data.size(); }
  bool next() { return finished() ? false : data[idx++]; }
  void skip() { idx = data.size(); }
  void append(vector<bool> v) { data.insert(data.end(), v.begin(), v.end()); }
};

struct VerifyChannel {
  int idx;
  DataSequence magicNumber;
  DataSequence data;
  int dataIdx;
  VerifyChannel(int idx, vector<bool> data) : idx(idx), data(data) {
    magicNumber = uint_to_vector(MAGIC_NUMBERS[idx], 32);
    data = vector<bool>(MAX_BIT);
    dataIdx = 0;
  }
  bool finished() const { return magicNumber.finished() && data.finished(); }

  bool next() {
    if (!data.finished())
      return data.next();
    if (!magicNumber.finished())
      return magicNumber.next();
    return false;
  }
};

struct PacketSender {
  mt19937 rng;
  vector<bool> C;
  vector<vector<bool>> history;
  PacketSender(vector<bool> C) : rng(SEED), C(C) {}

  vector<bool> send(vector<bool> A) {
    auto mask = rng();
    for (int i = 0; i < MAX_BIT; i++) {
      if (C[i])
        A[i] = 0;
      else
        A[i] = A[i] ^ ((mask >> i) & 1);
    }
    auto ret = send_packet(A);
    rng.seed(vector_to_uint(ret) ^ SEED);
    for (int i = 0; i < MAX_BIT; i++)
      ret[i] = ret[i] ^ ((mask >> i) & 1);
    history.push_back(ret);
    return ret;
  }
};

vector<vector<bool>> decode_packet(vector<vector<bool>> R) {
  mt19937 rng(SEED);

  for (auto &row : R) {
    auto mask = rng();
    rng.seed(vector_to_uint(row) ^ SEED);
    for (int i = 0; i < MAX_BIT; i++) {
      row[i] = row[i] ^ ((mask >> i) & 1);
    }
  }
  return R;
}

uint getFirst31ValidChannels(const vector<vector<bool>> R) {
  uint validChannels = (1u << MAX_BIT) - 1;
  for (int i = 0; i < MAX_BIT; i++) {
    if (R[i][i]) {
      validChannels &= ~(1u << i);
      continue;
    }
    int tot1 = 0;
    for (int j = 0; j < 31; j++)
      tot1 += R[j][i];
    if (tot1 != 15)
      validChannels &= ~(1u << i);
  }
  return validChannels;
}

void updateValidChannels(uint &validChannels, int idx,
                         const vector<bool> &row) {
  for (int i = 0; i < MAX_BIT; i++) {
    if (row[i] != ((MAGIC_NUMBERS[i] >> idx) & 1))
      validChannels &= ~(1u << i);
  }
}

void send_message(vector<bool> M, vector<bool> C) {
  init();
  M.push_back(true);
  auto sender = PacketSender(C);
  auto data = DataSequence(M);
  int channelNumber = vector_to_uint(
      vector<bool>{data.next(), data.next(), data.next(), data.next()});
  int channel = 0;
  for (;; channel++) {
    if (C[channel])
      continue;
    if (channelNumber-- == 0)
      break;
  }
  uint validChannels = 0;
  auto verify = VerifyChannel(channel, C);
  for (int i = 0; !data.finished() || !verify.finished(); i++) {
    vector<bool> A(MAX_BIT);
    for (int j = 0; j < MAX_BIT; j++) {
      if (C[j])
        continue;
      if (j == channel && !verify.finished()) {
        A[j] = verify.next();
        continue;
      }
      A[j] = data.next();
    }
    auto ret = sender.send(A);
    if (i == 31) {
      validChannels = getFirst31ValidChannels(sender.history);
    }
    if (i >= 31) {
      updateValidChannels(validChannels, i - 31, ret);
    }
    if (!verify.magicNumber.finished() &&
        __builtin_popcount(validChannels) == 1) {
      verify.magicNumber.skip();
    }
  }
}

vector<bool> receive_message(vector<vector<bool>> R) {
  init();
  R = decode_packet(R);
  uint validChannels = getFirst31ValidChannels(R);
  int verifyChannel = -1;
  int verifyDataLen;
  for (verifyDataLen = 31; verifyChannel == -1; verifyDataLen++) {
    updateValidChannels(validChannels, verifyDataLen - 31, R[verifyDataLen]);
    if (__builtin_popcount(validChannels) == 1) {
      verifyChannel = __builtin_ctz(validChannels);
    }
  }
  vector<bool> C;
  for (int i = 0; i < 31; i++)
    C.push_back(R[i][verifyChannel]);
  uint first4 = 0;
  for (int i = 0; i != verifyChannel; i++) {
    if (C[i])
      continue;
    first4++;
  }
  auto M = uint_to_vector(first4, 4);
  for (const auto &row : R) {
    for (int i = 0; i < MAX_BIT; i++) {
      if (C[i] || (i == verifyChannel && verifyDataLen > 0))
        continue;
      M.push_back(row[i]);
    }
    verifyDataLen--;
  }
  while (!M.back())
    M.pop_back();
  M.pop_back();
  return M;
}
