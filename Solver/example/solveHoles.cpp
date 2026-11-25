#include "nqueens.h"

namespace {

constexpr bool USE_COUNT_PASS = true;

int N;
unsigned allowM[16];  
int rowOf[16];        
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

void dfs(int k, unsigned cols, unsigned d1, unsigned d2) {
    const int r = rowOf[k];
    unsigned avail = allowM[k] & ~(cols | (d1 >> r) | (d2 >> (N - 1 - r)));

    if (k == N - 1) {
        if (countOnly) {
            cnt += __builtin_popcount(avail);
            return;
        }
        while (avail) {
            unsigned b = avail & (0u - avail);
            avail ^= b;
            cur[r] = __builtin_ctz(b);
            emit();
        }
        return;
    }

    const int s1 = r, s2 = N - 1 - r;
    while (avail) {
        unsigned b = avail & (0u - avail);
        avail ^= b;
        cur[r] = __builtin_ctz(b);
        dfs(k + 1, cols | b, d1 | (b << s1), d2 | (b << s2));
    }
}

void branch(int c, bool m) {
    const unsigned b = 1u << c;
    const int r = rowOf[0];
    cur[r] = c;
    mir = m;
    long long before = cnt;
    dfs(1, b, b << r, b << (N - 1 - r));
    if (countOnly && m) cnt += cnt - before;  
}

void runAll(bool sym) {
    const unsigned a = allowM[0];
    for (int c = 0; c < N; c++) {
        if (!((a >> c) & 1)) continue;
        if (sym) {
            if (2 * c < N - 1) branch(c, true);
            else if (2 * c == N - 1) branch(c, false);  
        } else {
            branch(c, false);
        }
    }
}

unsigned revMask(unsigned m) {
    unsigned r = 0;
    for (int i = 0; i < N; i++)
        if ((m >> i) & 1) r |= 1u << (N - 1 - i);
    return r;
}

}  // namespace

std::vector<std::vector<int>> solveHoles(int n, const std::vector<std::string>& grid) {
    std::vector<std::vector<int>> res;
    if (n <= 0 || n > 16) return res;
    if (n == 1) {
        if (grid[0][0] != 'H') res.push_back({0});
        return res;
    }
    if (n < 4) return res;  
    N = n;
    out = &res;

    unsigned am[16];
    bool sym = true;
    for (int r = 0; r < n; r++) {
        unsigned m = 0;
        for (int c = 0; c < n; c++)
            if (grid[r][c] != 'H') m |= 1u << c;
        if (!m) return res;  
        am[r] = m;
    }
    for (int r = 0; r < n; r++)
        if (revMask(am[r]) != am[r]) { sym = false; break; }

    int ord[16];
    for (int i = 0; i < n; i++) ord[i] = i;
    for (int i = 1; i < n; i++) {
        int x = ord[i];
        int px = __builtin_popcount(am[x]);
        int j = i - 1;
        while (j >= 0 && __builtin_popcount(am[ord[j]]) > px) {
            ord[j + 1] = ord[j];
            j--;
        }
        ord[j + 1] = x;
    }
    for (int k = 0; k < n; k++) {
        rowOf[k] = ord[k];
        allowM[k] = am[ord[k]];
    }

    if (USE_COUNT_PASS) {
        countOnly = true;
        cnt = 0;
        runAll(sym);
        if (cnt == 0) return res;
        res.reserve((size_t)cnt);
    }
    countOnly = false;
    runAll(sym);
    return res;
}