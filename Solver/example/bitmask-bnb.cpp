// Normal, Pre-placed, Holes : backtracking per baris dengan bitmask (kolom, diagonal, sel terlarang).
// Walled                    : branch and bound per segmen baris; satu segmen = satu antrean sel tanpa tembok.
#include <algorithm>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "nqueens.h"

namespace {

using Solutions = std::vector<std::vector<int>>;
using Board = std::vector<std::pair<int, int>>;

class RowSearch {
 public:
  RowSearch(int n, std::vector<unsigned> block, std::vector<int> fixed)
      : n_(n), full_((1u << n) - 1), block_(std::move(block)), fixed_(std::move(fixed)), cols_(n) {}

  Solutions run() {
    dfs(0, 0, 0, 0);
    return std::move(out_);
  }

  // Tanpa rintangan papan simetris: cari separuh kolom baris pertama, separuhnya lagi dicerminkan.
  Solutions runSymmetric() {
    for (int c = 0; c < (n_ + 1) / 2; ++c) {
      const size_t from = out_.size();
      const unsigned b = 1u << c;
      cols_[0] = c;
      dfs(1, b, b << 1, b >> 1);
      if (2 * c + 1 == n_) continue;
      const size_t to = out_.size();
      for (size_t i = from; i < to; ++i) {
        std::vector<int> mirrored = out_[i];
        for (int& x : mirrored) x = n_ - 1 - x;
        out_.push_back(std::move(mirrored));
      }
    }
    return std::move(out_);
  }

 private:
  const int n_;
  const unsigned full_;
  std::vector<unsigned> block_;
  std::vector<int> fixed_;
  std::vector<int> cols_;
  Solutions out_;

  void dfs(int r, unsigned col, unsigned d1, unsigned d2) {
    if (r == n_) {
      out_.push_back(cols_);
      return;
    }
    unsigned avail = full_ & ~(col | d1 | d2 | block_[r]);
    if (fixed_[r] >= 0) avail &= 1u << fixed_[r];
    while (avail) {
      const unsigned b = avail & -avail;
      avail ^= b;
      cols_[r] = __builtin_ctz(b);
      dfs(r + 1, col | b, (d1 | b) << 1, (d2 | b) >> 1);
    }
  }
};

class WalledSearch {
 public:
  WalledSearch(int n, const std::vector<std::string>& grid) : n_(n) {
    static const int DR[4] = {0, 1, 1, 1}, DC[4] = {1, 0, 1, -1};
    for (int k = 0; k < 4; ++k) {
      seg_[k].assign(n * n, -1);
      for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
          if (grid[r][c] == 'W' || seg_[k][r * n + c] >= 0) continue;
          const int id = segments_[k]++;
          for (int i = r, j = c; inside(i, j) && grid[i][j] != 'W'; i += DR[k], j += DC[k]) seg_[k][i * n + j] = id;
        }
      }
      used_[k].assign(segments_[k], 0);
      mark_[k].assign(segments_[k], 0);
      freeSegs_[k] = segments_[k];
    }
    rows_.resize(segments_[0]);
    for (int cell = 0; cell < n * n; ++cell)
      if (seg_[0][cell] >= 0) rows_[seg_[0][cell]].push_back(cell);
  }

  std::vector<Board> run() {
    decided_.assign(rows_.size(), 0);
    dfs(0);
    return std::move(best_);
  }

 private:
  const int n_;
  int segments_[4] = {0, 0, 0, 0};
  int freeSegs_[4];
  std::vector<int> seg_[4];
  std::vector<char> used_[4];
  std::vector<int> mark_[4];
  std::vector<char> decided_;
  int stamp_ = 0;
  std::vector<std::vector<int>> rows_;
  Board queens_;
  std::vector<Board> best_;
  int bestSize_ = 0;

  bool inside(int r, int c) const { return r >= 0 && c >= 0 && r < n_ && c < n_; }

  bool available(int cell) const {
    return !used_[1][seg_[1][cell]] && !used_[2][seg_[2][cell]] && !used_[3][seg_[3][cell]];
  }

  void toggle(int cell, char on) {
    for (int k = 1; k < 4; ++k) {
      used_[k][seg_[k][cell]] = on;
      freeSegs_[k] += on ? -1 : 1;
    }
  }

  // Pilih segmen baris tersempit (paling sedikit sel bebas) dan hitung batas atas sisa antrean:
  // baris, kolom, dan kedua diagonal yang masih bisa dipakai sel bebas.
  int scan(int& pick) {
    ++stamp_;
    pick = -1;
    int fewest = 1 << 30, rowsAlive = 0, alive[4] = {0, 0, 0, 0};
    for (size_t i = 0; i < rows_.size(); ++i) {
      if (decided_[i]) continue;
      int count = 0;
      for (int cell : rows_[i]) {
        if (!available(cell)) continue;
        ++count;
        for (int k = 1; k < 4; ++k) {
          int& mark = mark_[k][seg_[k][cell]];
          if (mark != stamp_) {
            mark = stamp_;
            ++alive[k];
          }
        }
      }
      if (!count) continue;
      ++rowsAlive;
      if (count < fewest) {
        fewest = count;
        pick = static_cast<int>(i);
      }
    }
    return std::min({rowsAlive, alive[1], alive[2], alive[3]});
  }

  void dfs(int cur) {
    int pick;
    if (cur + scan(pick) < bestSize_) return;
    if (pick < 0) {
      if (cur > bestSize_) {
        bestSize_ = cur;
        best_.clear();
      }
      best_.push_back(queens_);
      NQ_SNAPSHOT(queens_, "board with " + std::to_string(cur) + " queens");
      return;
    }
    decided_[pick] = 1;
    for (int cell : rows_[pick]) {
      if (!available(cell)) continue;
      toggle(cell, 1);
      queens_.emplace_back(cell / n_, cell % n_);
      dfs(cur + 1);
      queens_.pop_back();
      toggle(cell, 0);
    }
    dfs(cur);
    decided_[pick] = 0;
  }
};

}  // namespace

Solutions solveNQueens(int n) {
  return RowSearch(n, std::vector<unsigned>(n, 0), std::vector<int>(n, -1)).runSymmetric();
}

Solutions solvePreplaced(int n, const std::vector<std::pair<int, int>>& preplaced) {
  std::vector<unsigned> block(n, 0);
  std::vector<int> fixed(n, -1);
  for (const auto& [r, c] : preplaced) {
    if (fixed[r] >= 0 && fixed[r] != c) return {};
    fixed[r] = c;
  }
  for (int pr = 0; pr < n; ++pr) {
    if (fixed[pr] < 0) continue;
    const int pc = fixed[pr];
    for (int r = 0; r < n; ++r) {
      if (r == pr) continue;
      const int d = std::abs(r - pr);
      block[r] |= 1u << pc;
      if (pc - d >= 0) block[r] |= 1u << (pc - d);
      if (pc + d < n) block[r] |= 1u << (pc + d);
    }
  }
  for (int r = 0; r < n; ++r)
    if (fixed[r] >= 0 && (block[r] >> fixed[r] & 1u)) return {};
  return RowSearch(n, std::move(block), std::move(fixed)).run();
}

Solutions solveHoles(int n, const std::vector<std::string>& grid) {
  std::vector<unsigned> block(n, 0);
  for (int r = 0; r < n; ++r)
    for (int c = 0; c < n; ++c)
      if (grid[r][c] == 'H') block[r] |= 1u << c;
  return RowSearch(n, std::move(block), std::vector<int>(n, -1)).run();
}

std::vector<Board> solveWalled(int n, const std::vector<std::string>& grid) {
  return WalledSearch(n, grid).run();
}