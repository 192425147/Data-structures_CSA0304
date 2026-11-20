/* Q3: B-tree (CLRS, split on the way down), search with a disk-read counter, 5-million-key simulation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define MAXT 50
typedef struct BN { int n, leaf; int key[2 * MAXT - 1]; struct BN *c[2 * MAXT]; } BN;
static int T = 2;                         /* minimum degree: every node but the root has >= T-1 keys, <= 2T-1 keys */
static int VERBOSE = 0;
static long NODES;
BN *newnode(int leaf) { BN *x = calloc(1, sizeof *x); x->leaf = leaf; NODES++; return x; }
void split_child(BN *x, int i)            /* split the full child x->c[i] */
{
    BN *y = x->c[i], *z = newnode(y->leaf);
    if (VERBOSE) { printf("    split full node ["); for (int j = 0; j < y->n; j++) printf(j ? " %d" : "%d", y->key[j]); printf("]: median %d moves up\n", y->key[T - 1]); }
    z->n = T - 1;
    for (int j = 0; j < T - 1; j++) z->key[j] = y->key[j + T];
    if (!y->leaf) for (int j = 0; j < T; j++) z->c[j] = y->c[j + T];
    y->n = T - 1;
    for (int j = x->n; j >= i + 1; j--) x->c[j + 1] = x->c[j];
    x->c[i + 1] = z;
    for (int j = x->n - 1; j >= i; j--) x->key[j + 1] = x->key[j];
    x->key[i] = y->key[T - 1];
    x->n++;
}
void insert_nonfull(BN *x, int k)
{
    int i = x->n - 1;
    if (x->leaf) {
        while (i >= 0 && k < x->key[i]) { x->key[i + 1] = x->key[i]; i--; }
        x->key[i + 1] = k; x->n++;
    } else {
        while (i >= 0 && k < x->key[i]) i--;
        i++;
        if (x->c[i]->n == 2 * T - 1) { split_child(x, i); if (k > x->key[i]) i++; }
        insert_nonfull(x->c[i], k);
    }
}
BN *insert(BN *root, int k)
{
    if (!root) { root = newnode(1); root->key[0] = k; root->n = 1; return root; }
    if (root->n == 2 * T - 1) {
        BN *s = newnode(0); s->c[0] = root;
        if (VERBOSE) printf("   root is full -> tree grows one level\n");
        split_child(s, 0); insert_nonfull(s, k); return s;
    }
    insert_nonfull(root, k); return root;
}
/* search: counts one DISK READ for every node visited (the root is also read from disk) */
int search(BN *x, int k, long *reads)
{
    long r = 0;
    while (x) {
        r++;
        int i = 0; while (i < x->n && k > x->key[i]) i++;
        if (i < x->n && k == x->key[i]) { *reads = r; return 1; }
        if (x->leaf) break;
        x = x->c[i];
    }
    *reads = r; return 0;
}
void show(BN *x)
{
    printf("["); for (int i = 0; i < x->n; i++) printf(i ? " %d" : "%d", x->key[i]); printf("]");
    if (!x->leaf) { printf("{"); for (int i = 0; i <= x->n; i++) { if (i) printf(" "); show(x->c[i]); } printf("}"); }
}
int bheight(BN *x) { int h = 1; while (!x->leaf) { x = x->c[0]; h++; } return h; }    /* levels */
/* invariant check; returns number of keys or -1; depth of every leaf must be equal */
int LEAFDEPTH;
long check(BN *x, int depth, int isroot, long lo, long hi)
{
    if (x->n > 2 * T - 1 || (!isroot && x->n < T - 1) || (isroot && x->n < 1)) return -1;
    long tot = x->n;
    for (int i = 0; i < x->n; i++) { if (x->key[i] <= lo || x->key[i] >= hi) return -1; if (i && x->key[i - 1] >= x->key[i]) return -1; }
    if (x->leaf) { if (LEAFDEPTH < 0) LEAFDEPTH = depth; return depth == LEAFDEPTH ? tot : -1; }
    for (int i = 0; i <= x->n; i++) {
        long l = i ? x->key[i - 1] : lo, h = i < x->n ? x->key[i] : hi;
        long s = check(x->c[i], depth + 1, 0, l, h); if (s < 0) return -1; tot += s;
    }
    return tot;
}
void destroy(BN *x) { if (!x) return; if (!x->leaf) for (int i = 0; i <= x->n; i++) destroy(x->c[i]); free(x); }

/* ---- plain BST and AVL used for the node-visit comparison ---- */
typedef struct Nd { int key, h; struct Nd *l, *r; } Nd;
int ht(Nd *n) { return n ? n->h : 0; }
int mx(int a, int b) { return a > b ? a : b; }
void fx(Nd *n) { n->h = 1 + mx(ht(n->l), ht(n->r)); }
Nd *rrot(Nd *y) { Nd *x = y->l; y->l = x->r; x->r = y; fx(y); fx(x); return x; }
Nd *lrot(Nd *x) { Nd *y = x->r; x->r = y->l; y->l = x; fx(x); fx(y); return y; }
Nd *avl(Nd *n, int k)
{
    if (!n) { n = calloc(1, sizeof *n); n->key = k; n->h = 1; return n; }
    if (k < n->key) n->l = avl(n->l, k); else if (k > n->key) n->r = avl(n->r, k); else return n;
    fx(n); int b = ht(n->l) - ht(n->r);
    if (b > 1 && k < n->l->key) return rrot(n);
    if (b < -1 && k > n->r->key) return lrot(n);
    if (b > 1) { n->l = lrot(n->l); return rrot(n); }
    if (b < -1) { n->r = rrot(n->r); return lrot(n); }
    return n;
}
Nd *bst(Nd *root, int k) { Nd **p = &root; while (*p) { if (k == (*p)->key) return root; p = k < (*p)->key ? &(*p)->l : &(*p)->r; } *p =
    calloc(1, sizeof **p); (*p)->key = k; (*p)->h = 1; return root; }
long visits(Nd *x, int k) { long v = 0; while (x) { v++; if (k == x->key) return v; x = k < x->key ? x->l : x->r; } return v; }
int bst_levels(Nd *root)       /* iterative level count (random BST is shallow, ~50) */
{
    if (!root) return 0;
    int cap = 5000010; Nd **q = malloc(cap * sizeof *q); int h = 0, a = 0, b = 0; q[b++] = root;
    while (a < b) { int l = b - a; h++; for (int i = 0; i < l; i++) { Nd *x = q[a++]; if (x->l) q[b++] = x->l; if (x->r) q[b++] = x->r; } }
    free(q); return h;
}
void kill(Nd *x) { if (x) { kill(x->l); kill(x->r); free(x); } }
static unsigned long long rs = 88172645463325252ULL;
static unsigned rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (unsigned)(rs >> 11); }

int main(void)
{
    /* Tasks 25 and 34: keys 10 20 5 6 12 30 7 17 into a B-tree of minimum degree 2 (order 4) */
    int ks[] = {10, 20, 5, 6, 12, 30, 7, 17}; BN *r = NULL; T = 2; VERBOSE = 1;
    printf("[T25,T34] B-tree, minimum degree t=2 (2-3-4 tree), insertion keys 10 20 5 6 12 30 7 17\n");
    for (int i = 0; i < 8; i++) { printf(" insert %d\n", ks[i]); r = insert(r, ks[i]); printf("   tree: "); show(r); printf("\n"); }
    LEAFDEPTH = -1; printf(" invariants: keys=%ld (all leaves at the same depth, key counts within [t-1, 2t-1])\n", check(r, 1, 1, -1, 1L <<
        40));
    /* Task 36 on the small tree */
    printf("\n[T36 small tree] searches (reads = nodes visited)\n");
    int qs[] = {5, 30, 4, 18, 99};
    for (int i = 0; i < 5; i++) { long rd; int f = search(r, qs[i], &rd); printf(" search %-3d found=%d disk reads=%ld %s\n", qs[i], f, rd,
        qs[i] == 5 ? "(smallest)" : qs[i] == 30 ? "(largest)" : "(missing)"); }
    destroy(r); VERBOSE = 0; NODES = 0;

    /* Task 27 height formula */
    double n = 5e6;
    printf("\n[T27] height of a B-tree of order m=100 (t=50) holding n=5,000,000 keys\n");
    printf(" worst case (every node has the minimum): h <= log_t((n+1)/2) = log_50(%.1f) = %.3f edges -> at most %d levels (disk reads)\n",
           (n + 1) / 2, log((n + 1) / 2) / log(50), (int)floor(log((n + 1) / 2) / log(50)) + 1);
    printf(" best case (every node full, 99 keys): levels = ceil(log_100(n+1)) = %d\n", (int)ceil(log(n + 1) / log(100)));
    printf(" capacity of a full tree with L levels (99 keys/node, 100 children): L=3 -> %.0f keys, L=4 -> %.0f keys\n", pow(100, 3) - 1,
        pow(100, 4) - 1);
    printf(" binary comparison: ceil(log2(n+1)) = %d levels; AVL worst 1.44*log2(n+2)-0.33 = %.1f levels\n", (int)ceil(log2(n + 1)), 1.4405 *
        log2(n + 2) - 0.3277);

    /* Tasks 26, 28, 33, 35, 36 : build the real structures with 5,000,000 keys (even numbers 0..9,999,998, random insertion order) */
    enum { N = 5000000 };
    int *perm = malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) perm[i] = i;
    for (int i = N - 1; i > 0; i--) { int j = rnd() % (i + 1), x = perm[i]; perm[i] = perm[j]; perm[j] = x; }
    T = 50; BN *bt = NULL; NODES = 0;
    for (int i = 0; i < N; i++) bt = insert(bt, 2 * perm[i]);
    LEAFDEPTH = -1; long tot = check(bt, 1, 1, -1, 1L << 40);
    printf("\n[T28] B-tree t=50 built from %d random-order keys: levels=%d, nodes=%ld, avg keys per node=%.1f, invariant check keys=%ld %s\n",
           N, bheight(bt), NODES, (double)N / NODES, tot, tot == N ? "(OK)" : "(FAILED)");
    Nd *tb = NULL, *ta = NULL;
    for (int i = 0; i < N; i++) { tb = bst(tb, 2 * perm[i]); ta = avl(ta, 2 * perm[i]); }
    printf("      BST levels=%d, AVL levels=%d\n", bst_levels(tb), ht(ta));
    /* 100000 random successful searches */
    int Q = 100000; double sb = 0, sa = 0, sbt = 0; long maxb = 0, maxa = 0, maxbt = 0;
    for (int i = 0; i < Q; i++) {
        int k = 2 * (rnd() % N); long rd; search(bt, k, &rd); sbt += rd; if (rd > maxbt) maxbt = rd;
        long v = visits(ta, k); sa += v; if (v > maxa) maxa = v;
        v = visits(tb, k); sb += v; if (v > maxb) maxb = v;
    }
    printf("\n[T33,T35] %d random successful searches, one node = one disk block = one read of 10 ms\n", Q);
    printf(" %-6s %12s %10s %14s\n", "tree", "avg visits", "max", "avg time/search");
    printf(" %-6s %12.2f %10ld %11.1f ms\n", "BST", sb / Q, maxb, sb / Q * 10);
    printf(" %-6s %12.2f %10ld %11.1f ms\n", "AVL", sa / Q, maxa, sa / Q * 10);
    printf(" %-6s %12.2f %10ld %11.1f ms\n", "B-tree", sbt / Q, maxbt, sbt / Q * 10);
    /* Task 36: smallest, largest, missing */
    printf("\n[T36] boundary searches in the 5,000,000-key trees\n");
    int bk[] = {0, 2 * (N - 1), 1, 2 * N + 7, -5}; const char *nm[] = {"smallest 0", "largest 9999998", "missing 1 (inside range)", "missing 10000007 (above max)", "missing -5 (below min)"};
    for (int i = 0; i < 5; i++) { long rd; int f = search(bt, bk[i], &rd); printf(" %-30s found=%d B-tree reads=%ld (%ld ms)    AVL visits=%ld BST visits=%ld\n", nm[i], f, rd, rd * 10, visits(ta, bk[i]), visits(tb, bk[i])); }
    printf(" reference: even a perfectly balanced binary tree of 5M keys needs 23 reads = 230 ms per search\n");
    destroy(bt); kill(tb); kill(ta); free(perm);
    return 0;
}
