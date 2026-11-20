/* Q2: AVL insertion with rotation bookkeeping; comparison with a plain BST (tasks 13-16, 21-24) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef struct Nd { int key, h; struct Nd *l, *r; } Nd;
static long ROT[4];                 /* LL RR LR RL counts */
static long ROT_TOTAL;              /* a double rotation counts as 2 rotations */
static const char *LAST = "none";
int ht(Nd *n) { return n ? n->h : 0; }
int mx(int a, int b) { return a > b ? a : b; }
void fix(Nd *n) { n->h = 1 + mx(ht(n->l), ht(n->r)); }
int bf(Nd *n) { return n ? ht(n->l) - ht(n->r) : 0; }
Nd *rr(Nd *y) { Nd *x = y->l; y->l = x->r; x->r = y; fix(y); fix(x); ROT_TOTAL++; return x; }
Nd *rl(Nd *x) { Nd *y = x->r; x->r = y->l; y->l = x; fix(x); fix(y); ROT_TOTAL++; return y; }
Nd *avl_insert(Nd *n, int key)
{
    if (!n) { n = calloc(1, sizeof *n); n->key = key; n->h = 1; return n; }
    if (key < n->key) n->l = avl_insert(n->l, key);
    else if (key > n->key) n->r = avl_insert(n->r, key);
    else return n;
    fix(n);
    int b = bf(n);
    if (b > 1 && key < n->l->key) { ROT[0]++; LAST = "LL"; return rr(n); }
    if (b < -1 && key > n->r->key) { ROT[1]++; LAST = "RR"; return rl(n); }
    if (b > 1) { ROT[2]++; LAST = "LR"; n->l = rl(n->l); return rr(n); }
    if (b < -1) { ROT[3]++; LAST = "RL"; n->r = rr(n->r); return rl(n); }
    return n;
}
Nd *bst_insert(Nd *root, int key)       /* iterative plain BST */
{
    Nd **p = &root;
    while (*p) { if (key == (*p)->key) return root; p = key < (*p)->key ? &(*p)->l : &(*p)->r; }
    *p = calloc(1, sizeof **p); (*p)->key = key; (*p)->h = 1;
    return root;
}
int bst_height_nodes(Nd *t)             /* height counted in nodes on the longest path (leaf=1) */
{
    if (!t) return 0;
    /* iterative to survive a 100000-deep chain: BFS by levels */
    int cap = 1 << 20; Nd **q = malloc(cap * sizeof *q); int h = 0, t0 = 0, t1 = 0, lvl;
    q[t1++] = t;
    while (t0 < t1) { lvl = t1 - t0; h++; for (int i = 0; i < lvl; i++) { Nd *x = q[t0++]; if (x->l) q[t1++] = x->l; if (x->r) q[t1++] = x->r;
        if (t1 > cap - 2) { fprintf(stderr, "queue\n"); exit(1);} } }
    free(q); return h;
}
void show(Nd *t) { if (!t) { printf("-"); return; } if (!t->l && !t->r) { printf("%d", t->key); return; } printf("(%d ", t->key); show(t->l);
    printf(" "); show(t->r); printf(")"); }
void destroy(Nd *t) { if (t) { destroy(t->l); destroy(t->r); free(t); } }
void destroy_chain(Nd *t) { while (t) { Nd *nx = t->l ? t->l : t->r; /* only valid for chains */ free(t); t = nx; } }
/* verify: returns height, or -1 if any balance factor is outside {-1,0,1}, stored h wrong or BST order broken */
long NCHK, MAXABS;
int verify(Nd *t, long lo, long hi)
{
    if (!t) return 0;
    if (t->key <= lo || t->key >= hi) return -1;
    int a = verify(t->l, lo, t->key), b = verify(t->r, t->key, hi);
    if (a < 0 || b < 0) return -1;
    int d = a - b; if (d < 0) d = -d; if (d > MAXABS) MAXABS = d; NCHK++;
    if (d > 1 || t->h != 1 + mx(a, b)) return -1;
    return 1 + mx(a, b);
}
static double now(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return ts.tv_sec + ts.tv_nsec * 1e-9; }
static unsigned long long rs = 88172645463325252ULL;
static unsigned rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (unsigned)(rs >> 11); }

int main(void)
{
    /* Task 15: tree after every insertion, with rotation used */
    int pr[] = {10, 20, 30, 40, 50, 25}; Nd *r = NULL;
    printf("[T15] AVL insertions (tree printed as S-expression; '-' = empty)\n");
    for (int i = 0; i < 6; i++) {
        long before = ROT_TOTAL; LAST = "none";
        r = avl_insert(r, pr[i]);
        printf(" insert %2d -> rotation: %-4s (%ld rotation%s) tree: ", pr[i], LAST, ROT_TOTAL - before, ROT_TOTAL - before == 1 ? "" : "s");
            show(r); printf("   height(nodes)=%d\n", ht(r));
    }
    destroy(r); memset(ROT, 0, sizeof ROT); ROT_TOTAL = 0;

    /* Tasks 16, 21-24: n = 100000 */
    enum { N = 100000 };
    int *sorted = malloc(N * sizeof(int)), *perm = malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) sorted[i] = perm[i] = i + 1;
    for (int i = N - 1; i > 0; i--) { int j = rnd() % (i + 1), x = perm[i]; perm[i] = perm[j]; perm[j] = x; }
    printf("\n[T16,T21-T24] n = %d keys\n", N);
    double t0, t1;
    /* AVL sorted */
    Nd *as = NULL; memset(ROT, 0, sizeof ROT); ROT_TOTAL = 0;
    t0 = now(); for (int i = 0; i < N; i++) as = avl_insert(as, sorted[i]); t1 = now();
    double avl_sorted_ins = t1 - t0; long rs_tot = ROT_TOTAL, rs_c[4]; memcpy(rs_c, ROT, sizeof rs_c);
    /* AVL random */
    Nd *ar = NULL; memset(ROT, 0, sizeof ROT); ROT_TOTAL = 0;
    t0 = now(); for (int i = 0; i < N; i++) ar = avl_insert(ar, perm[i]); t1 = now();
    double avl_rand_ins = t1 - t0; long rr_tot = ROT_TOTAL, rr_c[4]; memcpy(rr_c, ROT, sizeof rr_c);
    /* BST sorted (degenerate) and random */
    Nd *bs = NULL; t0 = now(); for (int i = 0; i < N; i++) bs = bst_insert(bs, sorted[i]); t1 = now(); double bst_sorted_ins = t1 - t0;
    Nd *br = NULL; t0 = now(); for (int i = 0; i < N; i++) br = bst_insert(br, perm[i]); t1 = now(); double bst_rand_ins = t1 - t0;
    printf(" heights (levels on longest path): AVL sorted=%d AVL random=%d BST sorted=%d BST random=%d\n",
           ht(as), ht(ar), bst_height_nodes(bs), bst_height_nodes(br));
    printf(" rotations: AVL sorted total=%ld (LL=%ld RR=%ld LR=%ld RL=%ld)\n", rs_tot, rs_c[0], rs_c[1], rs_c[2], rs_c[3]);
    printf("             AVL random total=%ld (LL=%ld RR=%ld LR=%ld RL=%ld) = %.3f per insertion\n", rr_tot, rr_c[0], rr_c[1], rr_c[2],
        rr_c[3], (double)rr_tot / N);
    printf(" insertion time for %d keys (s): AVL sorted=%.4f AVL random=%.4f BST sorted=%.4f BST random=%.4f\n", N, avl_sorted_ins,
        avl_rand_ins, bst_sorted_ins, bst_rand_ins);
    /* search timing: 20000 random successful searches in each tree */
    int Q = 20000; int *qk = malloc(Q * sizeof(int)); for (int i = 0; i < Q; i++) qk[i] = 1 + rnd() % N;
    Nd *trees[4] = {as, ar, bs, br}; const char *nm[4] = {"AVL sorted", "AVL random", "BST sorted", "BST random"};
    double st[4]; double avgv[4];
    for (int k = 0; k < 4; k++) {
        long visits = 0; volatile int found = 0; t0 = now();
        for (int i = 0; i < Q; i++) { Nd *x = trees[k]; while (x) { visits++; if (qk[i] == x->key) { found++; break; } x = qk[i] < x->key ?
             x->l : x->r; } }
        t1 = now(); st[k] = (t1 - t0) / Q; avgv[k] = (double)visits / Q;
        if (found != Q) printf("search error\n");
    }
    printf(" search (average over %d random successful searches):\n", Q);
    for (int k = 0; k < 4; k++) printf("    %-11s nodes visited per search = %9.2f  time per search = %9.1f ns\n", nm[k], avgv[k], st[k] *
        1e9);
    /* Task 24: verify balance factors */
    NCHK = 0; MAXABS = 0; int v1 = verify(as, -1, 1L << 40); long c1 = NCHK, m1 = MAXABS;
    NCHK = 0; MAXABS = 0; int v2 = verify(ar, -1, 1L << 40); long c2 = NCHK, m2 = MAXABS;
    printf(" verify AVL sorted : %s, %ld nodes checked, largest |balance factor| = %ld\n", v1 > 0 ? "OK" : "VIOLATION", c1, m1);
    printf(" verify AVL random : %s, %ld nodes checked, largest |balance factor| = %ld\n", v2 > 0 ? "OK" : "VIOLATION", c2, m2);
    /* theory */
    printf(" bounds: log2(n+1)=%.2f (minimum possible levels = %d), 1.44*log2(n+2)-0.328=%.2f (AVL worst-case levels)\n",
           17.6096, 17, 1.4405 * 17.6096 - 0.3277);
    destroy(as); destroy(ar); destroy_chain(bs); destroy(br);
    free(sorted); free(perm); free(qk);
    return 0;
}
