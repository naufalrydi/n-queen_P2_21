// Normal N-Queens: backtracking dengan bitmask.
#include "nqueens.h"

static int N;
static unsigned FULL;
static int cur[16];
static std::vector<std::vector<int>>* out;
static bool mir;

static inline void emit() {
    out->emplace_back(cur, cur + N);
    if (mir) {
        std::vector<int> m(N);
        for (int i = 0; i < N; i++) m[i] = N - 1 - cur[i];
        out->push_back(std::move(m));
    }
}

static void dfs(int r, unsigned cols, unsigned ld, unsigned rd) {
    unsigned avail = FULL & ~(cols | ld | rd);
    if (r == N - 2) {
        while (avail) {
            unsigned b = avail & (0u - avail);
            avail ^= b;
            unsigned last = FULL & ~((cols | b) | (((ld | b) << 1) & FULL) | ((rd | b) >> 1));
            if (last) {
                cur[r] = __builtin_ctz(b);
                cur[r + 1] = __builtin_ctz(last);
                emit();
            }
        }
        return;
    }
    while (avail) {
        unsigned b = avail & (0u - avail);
        avail ^= b;
        cur[r] = __builtin_ctz(b);
        dfs(r + 1, cols | b, ((ld | b) << 1) & FULL, (rd | b) >> 1);
    }
}

std::vector<std::vector<int>> solveNQueens(int n) {
    std::vector<std::vector<int>> res;
    if (n <= 0) return res;
    if (n == 1) { res.push_back({0}); return res; }
    if (n < 4) return res;  // n = 2, 3 tidak punya solusi

    // jumlah solusi n = 4..12 sudah diketahui, jadi hasil di-reserve sekali
    static const int jumlah[13] = {0, 1, 0, 0, 2, 10, 4, 40, 92, 352, 724, 2680, 14200};
    if (n <= 12) res.reserve(jumlah[n]);

    N = n; FULL = (1u << n) - 1; out = &res;
    for (int c = 0; c < n / 2; c++) {
        unsigned b = 1u << c;
        cur[0] = c; mir = true;
        dfs(1, b, (b << 1) & FULL, b >> 1);
    }
    if (n % 2 == 1) {  // kolom tengah dicari penuh, tanpa cermin
        unsigned b = 1u << (n / 2);
        cur[0] = n / 2; mir = false;
        dfs(1, b, (b << 1) & FULL, b >> 1);
    }
    return res;
}