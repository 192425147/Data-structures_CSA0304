/* Q7: dictionary of 200,000 (and 2,000,000) synthetic words.
   Chosen design: compact trie (first-child / next-sibling, siblings kept alphabetical, 12 bytes per node).
   Reference structures: sorted array + binary search, AVL tree of strings, 26-pointer trie (memory computed from the same node count). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
typedef struct { uint32_t child, sib; unsigned char c, end; } CN;            /* sizeof = 12 */
static CN *nodes; static uint32_t nn, ncap;
static uint32_t newnode(unsigned char c) { if (nn == ncap) { ncap = ncap ? ncap * 2 : 1024; nodes = realloc(nodes, ncap * sizeof(CN)); }
    nodes[nn] = (CN){0, 0, c, 0}; return nn++; }
static void trie_init(void) { free(nodes); nodes = NULL; nn = ncap = 0; newnode(0); }
static void trie_insert(const char *w)
{
    uint32_t cur = 0;
    for (const char *p = w; *p; p++) {
        uint32_t *link = &nodes[cur].child;
        while (*link && nodes[*link].c < (unsigned char)*p) link = &nodes[*link].sib;
        if (*link && nodes[*link].c == (unsigned char)*p) { cur = *link; continue; }
        uint32_t nxt = *link; uint32_t id = newnode(*p);          /* newnode may realloc: recompute the link afterwards */
        /* recompute link position by walking again (realloc invalidated the pointer) */
        link = &nodes[cur].child; while (*link && nodes[*link].c < (unsigned char)*p) link = &nodes[*link].sib;
        nodes[id].sib = nxt; *link = id; cur = id;
    }
    nodes[cur].end = 1;
}
static int trie_find(const char *w) { uint32_t cur = 0; for (const char *p = w; *p; p++) { uint32_t c = nodes[cur].child; while (c &&
    nodes[c].c < (unsigned char)*p) c = nodes[c].sib; if (!c || nodes[c].c != (unsigned char)*p) return 0; cur = c; } return nodes[cur].end; }
static int trie_node_for(const char *w, uint32_t *out) { uint32_t cur = 0; for (const char *p = w; *p; p++) { uint32_t c = nodes[cur].child;
    while (c && nodes[c].c < (unsigned char)*p) c = nodes[c].sib; if (!c || nodes[c].c != (unsigned char)*p) return 0; cur = c; } *out = cur;
    return 1; }
/* result collection */
#define MAXR 4096
static char res[MAXR][40]; static int nres, lim; static char pbuf[64];
static void dfs_all(uint32_t n, int len)
{
    if (nres >= lim) return;
    if (n && nodes[n].end) { pbuf[len] = 0; strcpy(res[nres++], pbuf); }
    for (uint32_t c = nodes[n].child; c && nres < lim; c = nodes[c].sib) { pbuf[len] = nodes[c].c; dfs_all(c, len + 1); }
}
static int autocomplete(const char *p, int limit)
{
    nres = 0; lim = limit; uint32_t n; if (!trie_node_for(p, &n)) return 0;
    strcpy(pbuf, p); int len = (int)strlen(p);
    if (nodes[n].end && n) { strcpy(res[nres++], p); }
    for (uint32_t c = nodes[n].child; c && nres < lim; c = nodes[c].sib) { pbuf[len] = nodes[c].c; dfs_all(c, len + 1); }
    return nres;
}
/* range listing lo <= w <= hi, alphabetical; prunes subtrees entirely outside the range */
static const char *LO, *HI;
static void dfs_range(uint32_t n, int len)
{
    for (uint32_t c = nodes[n].child; c && nres < lim; c = nodes[c].sib) {
        pbuf[len] = nodes[c].c; pbuf[len + 1] = 0; int L = len + 1;
        if (strncmp(pbuf, HI, L) > 0) return;                          /* this and all later siblings are above hi */
        if (strncmp(pbuf, LO, L) < 0) continue;                        /* whole subtree below lo */
        if (nodes[c].end && strcmp(pbuf, LO) >= 0 && strcmp(pbuf, HI) <= 0) strcpy(res[nres++], pbuf);
        dfs_range(c, L);
    }
}
static int range_list(const char *lo, const char *hi, int limit) { nres = 0; lim = limit; LO = lo; HI = hi; pbuf[0] = 0; dfs_range(0, 0);
    return nres; }
/* spell suggestions: all words within edit distance <= 1 of q, by DFS with a Levenshtein row */
static const char *Q; static int QL;
static void dfs_edit(uint32_t n, int len, const int *prev)
{
    for (uint32_t c = nodes[n].child; c && nres < lim; c = nodes[c].sib) {
        int row[40]; row[0] = prev[0] + 1; int mn = row[0];
        for (int j = 1; j <= QL; j++) {
            int cost = (Q[j - 1] == (char)nodes[c].c) ? 0 : 1;
            int v = prev[j - 1] + cost; if (prev[j] + 1 < v) v = prev[j] + 1; if (row[j - 1] + 1 < v) v = row[j - 1] + 1;
            row[j] = v; if (v < mn) mn = v;
        }
        pbuf[len] = nodes[c].c;
        if (nodes[c].end && row[QL] <= 1) { pbuf[len + 1] = 0; strcpy(res[nres++], pbuf); }
        if (mn <= 1) dfs_edit(c, len + 1, row);
    }
}
static int spell(const char *q, int limit) { Q = q; QL = (int)strlen(q); nres = 0; lim = limit; int row[40]; for (int j = 0; j <= QL; j++)
    row[j] = j; dfs_edit(0, 0, row); return nres; }
static int within1(const char *a, const char *b)
{
    int la = strlen(a), lb = strlen(b); if (la - lb > 1 || lb - la > 1) return 0;
    int i = 0; while (a[i] && b[i] && a[i] == b[i]) i++;
    if (la == lb) return strcmp(a + i + 1, b + i + 1) == 0;
    if (la > lb) return strcmp(a + i + 1, b + i) == 0;
    return strcmp(a + i, b + i + 1) == 0;
}
/* ----- AVL of strings (reference) ----- */
typedef struct A { char *s; int h; struct A *l, *r; } A;
static int ht(A *n) { return n ? n->h : 0; } static int mx(int a, int b) { return a > b ? a : b; }
static void fx(A *n) { n->h = 1 + mx(ht(n->l), ht(n->r)); }
static A *ar(A *y) { A *x = y->l; y->l = x->r; x->r = y; fx(y); fx(x); return x; }
static A *al(A *x) { A *y = x->r; x->r = y->l; y->l = x; fx(x); fx(y); return y; }
static A *avl_ins(A *n, char *s)
{
    if (!n) { n = calloc(1, sizeof *n); n->s = s; n->h = 1; return n; }
    int c = strcmp(s, n->s); if (c < 0) n->l = avl_ins(n->l, s); else if (c > 0) n->r = avl_ins(n->r, s); else return n;
    fx(n); int b = ht(n->l) - ht(n->r);
    if (b > 1 && strcmp(s, n->l->s) < 0) return ar(n);
    if (b < -1 && strcmp(s, n->r->s) > 0) return al(n);
    if (b > 1) { n->l = al(n->l); return ar(n); }
    if (b < -1) { n->r = ar(n->r); return al(n); }
    return n;
}
static int avl_find(A *n, const char *s) { while (n) { int c = strcmp(s, n->s); if (!c) return 1; n = c < 0 ? n->l : n->r; } return 0; }
static void avl_free(A *n) { if (n) { avl_free(n->l); avl_free(n->r); free(n); } }
/* ----- helpers ----- */
static unsigned long long rs = 88172645463325252ULL;
static unsigned rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (unsigned)(rs >> 11); }
static double now(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return ts.tv_sec + ts.tv_nsec * 1e-9; }
static int cmps(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }
static char *gen(void)
{
    static const char *C = "bcdfghjklmnprstvyz", *V = "aeiou"; char b[40]; int n = 0, syl = 2 + rnd() % 4;
    for (int i = 0; i < syl; i++) { b[n++] = C[rnd() % 18]; b[n++] = V[rnd() % 5]; if (rnd() % 3 == 0) b[n++] = C[rnd() % 18]; }
    b[n] = 0; return strdup(b);
}
static int lower_bound(char **a, int n, const char *k) { int lo = 0, hi = n; while (lo < hi) { int m = (lo + hi) / 2; if (strcmp(a[m], k) < 0)
    lo = m + 1; else hi = m; } return lo; }
static int lower_bound_prefix(char **a, int n, const char *p) { int lo = 0, hi = n, L = strlen(p); while (lo < hi) { int m = (lo + hi) / 2; if
    (strncmp(a[m], p, L) < 0) lo = m + 1; else hi = m; } return lo; }

static void run(int N, int verify)
{
    printf("\n================ dictionary of %d words ================\n", N);
    char **a = malloc((size_t)(N * 1.3) * sizeof *a); int m = 0, target = (int)(N * 1.25);
    char **tmp = malloc((size_t)target * sizeof *tmp); for (int i = 0; i < target; i++) tmp[i] = gen();
    qsort(tmp, target, sizeof *tmp, cmps);
    int u = 0; for (int i = 0; i < target; i++) { if (u && !strcmp(tmp[u - 1], tmp[i])) free(tmp[i]); else tmp[u++] = tmp[i]; }
    /* randomly keep exactly N words (drop the surplus at random positions so the dictionary is not biased) */
    char *keep = calloc(u, 1); int k = 0; while (k < N) { int j = rnd() % u; if (!keep[j]) { keep[j] = 1; k++; } }
    for (int i = 0; i < u; i++) { if (keep[i]) a[m++] = tmp[i]; else free(tmp[i]); } free(tmp); free(keep);
    long chars = 0; int maxl = 0; for (int i = 0; i < m; i++) { int L = strlen(a[i]); chars += L; if (L > maxl) maxl = L; }
    printf(" words=%d, average length=%.2f, longest=%d (sorted already, unique)\n", m, (double)chars / m, maxl);
    /* build */
    double t0 = now(); trie_init(); for (int i = 0; i < m; i++) trie_insert(a[i]); double t1 = now();
    double build_trie = t1 - t0;
    A *avl = NULL; t0 = now(); for (int i = 0; i < m; i++) avl = avl_ins(avl, a[i]); t1 = now(); double build_avl = t1 - t0;
    size_t trie_bytes = (size_t)nn * sizeof(CN), trie26 = (size_t)nn * 216, arr_bytes = 0, avl_bytes = 0;
    for (int i = 0; i < m; i++) { arr_bytes += strlen(a[i]) + 1 + 8; avl_bytes += strlen(a[i]) + 1 + 32; }
    printf(" nodes=%u (%.2f per word)\n", nn, (double)nn / m);
    printf(" memory (MB): compact trie=%.1f (12 B/node) | 26-pointer trie=%.1f (216 B/node, computed) | sorted array=%.1f | AVL of strings=%.1f\n",
           trie_bytes / 1e6, trie26 / 1e6, arr_bytes / 1e6, avl_bytes / 1e6);
    printf(" build time (s): compact trie from sorted list=%.3f AVL=%.3f\n", build_trie, build_avl);
    /* weekly update: 1000 new words inserted into the existing trie */
    t0 = now(); for (int i = 0; i < 1000; i++) { char *w = gen(); trie_insert(w); free(w); } t1 = now();
    printf(" weekly update: inserting 1000 new words into the trie = %.2f ms (%.2f us per word)\n", (t1 - t0) * 1e3, (t1 - t0) * 1e6 / 1000);
    /* rebuild the trie so that every later test works on exactly the dictionary a[] */
    trie_init(); for (int i = 0; i < m; i++) trie_insert(a[i]);

    /* ---- verification on a sample of 1,000 words ---- */
    if (verify) {
        int S = 1000, bad_exact = 0, bad_missing = 0, bad_auto = 0, bad_range = 0, bad_spell = 0, found_typo = 0; long spell_total = 0;
        for (int i = 0; i < S; i++) {
            const char *w = a[rnd() % m];
            if (!trie_find(w)) bad_exact++;
            char t[48]; strcpy(t, w); int L = strlen(t); t[rnd() % L] = 'x'; t[L] = 0;          /* a mistyped copy (may or may not be a word)
                */
            int in_arr = a[lower_bound(a, m, t)] && lower_bound(a, m, t) < m && !strcmp(a[lower_bound(a, m, t)], t);
            if (trie_find(t) != in_arr) bad_missing++;
            /* autocomplete: first 10 words with a 2-4 letter prefix versus the sorted array */
            char pf[8]; int PL = 2 + rnd() % 3; if (PL > L) PL = L; memcpy(pf, w, PL); pf[PL] = 0;
            int n1 = autocomplete(pf, 10); int s = lower_bound_prefix(a, m, pf); int n2 = 0; int same = 1;
            for (int j = s; j < m && n2 < 10 && !strncmp(a[j], pf, PL); j++, n2++) if (n2 >= n1 || strcmp(res[n2], a[j])) same = 0;
            if (n1 != n2 || !same) bad_auto++;
            /* range: from w to the word 30 positions later */
            int s2 = lower_bound(a, m, w), e2 = s2 + 30 < m ? s2 + 30 : m - 1;
            int n3 = range_list(w, a[e2], 1000); if (n3 != e2 - s2 + 1) bad_range++; else for (int j = 0; j < n3; j++) if (strcmp(res[j], a[s2
                + j])) { bad_range++; break; }
            /* spell: mistyped word, compare with brute force over the whole dictionary */
            int n4 = spell(t, 200); int bf = 0, ok = 1; for (int j = 0; j < m; j++) if (within1(t, a[j])) { if (bf >= n4 || strcmp(res[bf],
                a[j])) ok = 0; bf++; }
            if (!ok || bf != n4) bad_spell++;
            spell_total += n4;
            if (n4 > 0) found_typo++;
        }
        printf(" VERIFY on a sample of %d words (compared with brute force over the sorted array):\n", S);
        printf("   exact look-up of existing words: %d wrong | mistyped-word look-up: %d wrong | autocomplete(first 10): %d wrong | range listing (31 words): %d wrong | spell suggestions (edit distance <=1): %d wrong\n",
               bad_exact, bad_missing, bad_auto, bad_range, bad_spell);
        printf("   spell check: mistyped words that received >=1 suggestion = %d of %d (average %.2f suggestions)\n", found_typo, S,
            (double)spell_total / S);
    }
    /* ---- response time ---- */
    int Qn = 20000; char **qs = malloc(Qn * sizeof *qs); char **pq = malloc(Qn * sizeof *pq); char **ms = malloc(2000 * sizeof *ms);
    for (int i = 0; i < Qn; i++) { qs[i] = strdup(a[rnd() % m]); int L = strlen(qs[i]); int PL = 2 + rnd() % 3; if (PL > L) PL = L; pq[i] =
        strndup(qs[i], PL); }
    for (int i = 0; i < 2000; i++) { ms[i] = strdup(a[rnd() % m]); ms[i][rnd() % strlen(ms[i])] = 'x'; }
    volatile long sink = 0;
    t0 = now(); for (int i = 0; i < Qn; i++) sink += trie_find(qs[i]); t1 = now(); double e_trie = (t1 - t0) / Qn;
    t0 = now(); for (int i = 0; i < Qn; i++) { int p = lower_bound(a, m, qs[i]); sink += (p < m && !strcmp(a[p], qs[i])); } t1 = now(); double
        e_arr = (t1 - t0) / Qn;
    t0 = now(); for (int i = 0; i < Qn; i++) sink += avl_find(avl, qs[i]); t1 = now(); double e_avl = (t1 - t0) / Qn;
    t0 = now(); for (int i = 0; i < Qn; i++) sink += autocomplete(pq[i], 10); t1 = now(); double ac_trie = (t1 - t0) / Qn;
    t0 = now(); for (int i = 0; i < Qn; i++) { int s = lower_bound_prefix(a, m, pq[i]); int L = strlen(pq[i]), c = 0; for (int j = s; j < m &&
        c < 10 && !strncmp(a[j], pq[i], L); j++) c++; sink += c; } t1 = now(); double ac_arr = (t1 - t0) / Qn;
    long rtot = 0; t0 = now(); for (int i = 0; i < 2000; i++) { int s = lower_bound(a, m, qs[i]); int e = s + 100 < m ? s + 100 : m - 1; rtot
        += range_list(qs[i], a[e], 1000); } t1 = now(); double r_trie = (t1 - t0) / 2000;
    t0 = now(); for (int i = 0; i < 2000; i++) { int s = lower_bound(a, m, qs[i]); int e = s + 100 < m ? s + 100 : m - 1; int c = 0; for (int j
        = s; j <= e; j++) { sink += a[j][0]; c++; } sink += c; } t1 = now(); double r_arr = (t1 - t0) / 2000;
    long stot = 0; t0 = now(); for (int i = 0; i < 2000; i++) stot += spell(ms[i], 200); t1 = now(); double sp_trie = (t1 - t0) / 2000;
    int SB = 100; t0 = now(); for (int i = 0; i < SB; i++) for (int j = 0; j < m; j++) sink += within1(ms[i], a[j]); t1 = now(); double sp_arr
        = (t1 - t0) / SB;
    printf(" response time per operation (average):\n");
    printf("   exact look-up         : compact trie %8.0f ns | sorted array %8.0f ns | AVL of strings %8.0f ns\n", e_trie * 1e9, e_arr * 1e9,
        e_avl * 1e9);
    printf("   autocomplete (10)     : compact trie %8.0f ns | sorted array %8.0f ns\n", ac_trie * 1e9, ac_arr * 1e9);
    printf("   range listing (~100) : compact trie %8.1f us | sorted array %8.1f us    (avg %.0f words returned)\n", r_trie * 1e6, r_arr * 1e6,
        (double)rtot / 2000);
    printf("   spell suggestions (<=1 edit): compact trie %8.1f us (avg %.1f suggestions) | scan every word %8.1f us\n", sp_trie * 1e6,
        (double)stot / 2000, sp_arr * 1e6);
    (void)sink;
    avl_free(avl); for (int i = 0; i < m; i++) free(a[i]); free(a);
    for (int i = 0; i < Qn; i++) { free(qs[i]); free(pq[i]); } for (int i = 0; i < 2000; i++) free(ms[i]); free(qs); free(pq); free(ms);
}
int main(void)
{
    printf("sizeof(CN) = %zu bytes per compact node\n", sizeof(CN));
    /* small worked example for the report */
    trie_init(); const char *ex[] = {"cat", "car", "card", "care", "dog", "do"}; for (int i = 0; i < 6; i++) trie_insert(ex[i]);
    printf("example words: cat car card care dog do -> nodes=%u (incl. root)\n", nn);
    int n = autocomplete("car", 10); printf(" autocomplete(car):"); for (int i = 0; i < n; i++) printf(" %s", res[i]); printf("\n");
    n = range_list("car", "dog", 10); printf(" range [car, dog]:"); for (int i = 0; i < n; i++) printf(" %s", res[i]); printf("\n");
    n = spell("cax", 10); printf(" spell(cax):"); for (int i = 0; i < n; i++) printf(" %s", res[i]); printf("\n");
    n = spell("cr", 10); printf(" spell(cr):"); for (int i = 0; i < n; i++) printf(" %s", res[i]); printf("\n");
    run(1000, 1);
    run(200000, 1);
    run(2000000, 0);
    return 0;
}
