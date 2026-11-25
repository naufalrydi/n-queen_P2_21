#include "nqueens.h"
namespace {

constexpr bool USE_COUNT_PASS = false;

int N;
unsigned allowM[16];
int cur[16];
std::vector<std::vector<int>>* out;
long long cnt;
bool countOnly;
bool mir;

inline void emit() {
    out->emplace_back(cur, cur + N);
    if (mir) {
        out->emplace_back(cur, cur + N);
        for (int& x : out->back()) x = N - 1 - x;
    }
}

void dfs(unsigned rem, unsigned cols, unsigned d1, unsigned d2) {
    int bestR = -1, bestC = 99;
    unsigned bestA = 0;
    for (unsigned t = rem; t; t &= t - 1) {
        const int r = __builtin_ctz(t);
        unsigned a = allowM[r] & ~(cols | (d1 >> r) | (d2 >> (N - 1 - r)));
        if (!a) return;
        int pc = __builtin_popcount(a);
        if (pc < bestC) { bestC = pc; bestR = r; bestA = a; }
    }
    const int r = bestR;
    const unsigned rem2 = rem & ~(1u << r);
    if (!rem2) {
        if (countOnly) { cnt += bestC; return; }
        while (bestA) {
            unsigned b = bestA & (0u - bestA);
            bestA ^= b;
            cur[r] = __builtin_ctz(b);
            emit();
        }
        return;
    }
    const int s1 = r, s2 = N - 1 - r;
    while (bestA) {
        unsigned b = bestA & (0u - bestA);
        bestA ^= b;
        cur[r] = __builtin_ctz(b);
        dfs(rem2, cols | b, d1 | (b << s1), d2 | (b << s2));
    }
}

unsigned FULL;
void fdfs(int r, unsigned cols, unsigned ld, unsigned rd) {
    unsigned avail = FULL & ~(cols | ld | rd);
    if (r == N - 3) {
        while (avail) {
            unsigned b = avail & (0u - avail);
            avail ^= b;
            unsigned c2 = cols | b, l2 = ((ld | b) << 1) & FULL, r2 = (rd | b) >> 1;
            unsigned avail2 = FULL & ~(c2 | l2 | r2);
            while (avail2) {
                unsigned b2 = avail2 & (0u - avail2);
                avail2 ^= b2;
                unsigned last = FULL & ~((c2 | b2) | (((l2 | b2) << 1) & FULL) | ((r2 | b2) >> 1));
                if (last) {
                    if (countOnly) { cnt++; continue; }
                    cur[r] = __builtin_ctz(b); cur[r + 1] = __builtin_ctz(b2); cur[r + 2] = __builtin_ctz(last);
                    emit();
                }
            }
        }
        return;
    }
    while (avail) {
        unsigned b = avail & (0u - avail);
        avail ^= b;
        cur[r] = __builtin_ctz(b);
        fdfs(r + 1, cols | b, ((ld | b) << 1) & FULL, (rd | b) >> 1);
    }
}
void runFast() {
    for (int c = 0; 2 * c <= N - 1; c++) {
        const unsigned b = 1u << c;
        const bool m = (2 * c < N - 1);
        cur[0] = c; mir = m;
        long long before = cnt;
        fdfs(1, b, (b << 1) & FULL, b >> 1);
        if (countOnly && m) cnt += cnt - before;
    }
}
unsigned revMask(unsigned m) {
    unsigned r = 0;
    for (int i = 0; i < N; i++) if ((m >> i) & 1) r |= 1u << (N - 1 - i);
    return r;
}
void runAll(bool sym) {
    int r0 = 0, bp = 99;
    for (int r = 0; r < N; r++) { int p = __builtin_popcount(allowM[r]); if (p < bp) { bp = p; r0 = r; } }
    const unsigned a = allowM[r0];
    const unsigned rem = ((1u << N) - 1) & ~(1u << r0);
    for (int c = 0; c < N; c++) {
        if (!((a >> c) & 1)) continue;
        bool m = false;
        if (sym) { if (2 * c < N - 1) m = true; else if (2 * c > N - 1) continue; }
        const unsigned b = 1u << c;
        cur[r0] = c; mir = m;
        long long before = cnt;
        dfs(rem, b, b << r0, b << (N - 1 - r0));
        if (countOnly && m) cnt += cnt - before;
    }
}
}
std::vector<std::vector<int>> solveHoles(int n, const std::vector<std::string>& grid) {
    std::vector<std::vector<int>> res;
    if (n <= 0 || n > 16) return res;
    if (n == 1) { if (grid[0][0] != 'H') res.push_back({0}); return res; }
    if (n < 4) return res;
    N = n; out = &res;
    bool sym = true, noHole = true;
    for (int r = 0; r < n; r++) {
        unsigned m = 0;
        for (int c = 0; c < n; c++) if (grid[r][c] != 'H') m |= 1u << c;
        if (!m) return res;
        allowM[r] = m;
        if (m != (1u << n) - 1) noHole = false;
    }
    FULL = (1u << n) - 1;
    for (int r = 0; r < n; r++) if (revMask(allowM[r]) != allowM[r]) { sym = false; break; }
    if (USE_COUNT_PASS) {
        countOnly = true; cnt = 0; if (noHole) runFast(); else runAll(sym);
        if (cnt == 0) return res;
        res.reserve((size_t)cnt);
    }
    countOnly = false;
    if (noHole) runFast(); else runAll(sym);
    return res;
}
