// Preplaced N-Queens: backtracking dengan bitmask.
#include "nqueens.h"

static int N, lastFree;
static unsigned FULL;
static int cur[16];
static unsigned forb[16];
static bool fixedRow[16];
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
    while (fixedRow[r]) {          // baris terkunci: cuma geser diagonal
        ld = (ld << 1) & FULL;
        rd >>= 1;
        r++;
    }
    unsigned avail = FULL & ~(cols | ld | rd | forb[r]);
    if (r == lastFree) {           // sisa kolom pasti tinggal satu
        if (avail) { cur[r] = __builtin_ctz(avail); emit(); }
        return;
    }
    while (avail) {
        unsigned b = avail & (0u - avail);
        avail ^= b;
        cur[r] = __builtin_ctz(b);
        dfs(r + 1, cols | b, ((ld | b) << 1) & FULL, (rd | b) >> 1);
    }
}

std::vector<std::vector<int>> solvePreplaced(int n, const std::vector<std::pair<int, int>>& pre) {
    std::vector<std::vector<int>> res;
    if (n <= 0) return res;
    N = n; FULL = (1u << n) - 1; out = &res; mir = false;

    unsigned cols = 0;
    for (int i = 0; i < n; i++) { fixedRow[i] = false; forb[i] = 0; }
    fixedRow[n] = false;           // penjaga (array berukuran 16, n <= 12)

    for (const auto& p : pre) {
        int r = p.first, c = p.second;
        if (r < 0 || r >= n || c < 0 || c >= n) return res;
        if (fixedRow[r] || (cols >> c & 1u)) return res;   // bentrok baris/kolom
        fixedRow[r] = true; cur[r] = c; cols |= 1u << c;
    }
    for (const auto& p : pre) {    // diagonal ratu terkunci
        for (int r = 0; r < n; r++) {
            int d = r - p.first;
            if (d == 0) continue;
            for (int s = -1; s <= 1; s += 2) {
                int c = p.second + s * d;
                if (c < 0 || c >= n) continue;
                if (fixedRow[r]) { if (cur[r] == c) return res; }  // saling menyerang
                else forb[r] |= 1u << c;
            }
        }
    }

    lastFree = -1;
    for (int r = 0; r < n; r++) if (!fixedRow[r]) lastFree = r;
    if (lastFree < 0) {            // semua baris sudah terkunci
        res.emplace_back(cur, cur + n);
        return res;
    }

    if (pre.empty() && n >= 4) {   // P = 0: sama seperti Normal, pakai cermin
        static const int jumlah[13] = {0, 1, 0, 0, 2, 10, 4, 40, 92, 352, 724, 2680, 14200};
        if (n <= 12) res.reserve(jumlah[n]);
        for (int c = 0; c < n / 2; c++) {
            unsigned b = 1u << c;
            cur[0] = c; mir = true;
            dfs(1, b, (b << 1) & FULL, b >> 1);
        }
        if (n % 2 == 1) {
            unsigned b = 1u << (n / 2);
            cur[0] = n / 2; mir = false;
            dfs(1, b, (b << 1) & FULL, b >> 1);
        }
        return res;
    }
    dfs(0, cols, 0, 0);
    return res;
}