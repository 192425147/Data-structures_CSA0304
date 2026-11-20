/* Q5: splay tree (bottom-up, with parent pointers) versus AVL; step trace, workloads, worst cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef struct S { int key; struct S *l, *r, *p; } S;
static S *root; static long ROT; static int TRACE;
static void rot_up(S *x)                       /* rotate x above its parent */
{
    S *p = x->p, *g = p->p;
    if (p->l == x) { p->l = x->r; if (x->r) x->r->p = p; x->r = p; } else { p->r = x->l; if (x->l) x->l->p = p; x->l = p; }
    p->p = x; x->p = g;
    if (g) { if (g->l == p) g->l = x; else g->r = x; } else root = x;
    ROT++;
}
static void splay(S *x)
{
    while (x->p) {
        S *p = x->p, *g = p->p;
        if (!g) { if (TRACE) printf("     zig      at %d (parent %d is the root)\n", x->key, p->key); rot_up(x); }
        else if ((g->l == p) == (p->l == x)) { if (TRACE) printf("      zig-zig at %d (parent %d, grandparent %d): rotate parent first\n",
            x->key, p->key, g->key); rot_up(p); rot_up(x); }
        else { if (TRACE) printf("     zig-zag at %d (parent %d, grandparent %d): rotate x twice\n", x->key, p->key, g->key); rot_up(x);
            rot_up(x); }
    }
}
/* access: returns nodes visited on the search path (depth+1) ; splays the last node touched (also for a miss) */
static int access_key(int k, int *found)
{
    S *x = root, *last = NULL; int v = 0;
    while (x) { v++; last = x; if (k == x->key) { *found = 1; break; } x = k < x->key ? x->l : x->r; }
    if (x == NULL) *found = 0;
    if (last) splay(last);
    return v;
}
static void insert_key(int k)                  /* plain BST insert, then splay the new node to the root */
{
    S *n = calloc(1, sizeof *n); n->key = k;
    if (!root) { root = n; return; }
    S *x = root, *par = NULL; while (x) { par = x; x = k < x->key ? x->l : x->r; }
    n->p = par; if (k < par->key) par->l = n; else par->r = n; splay(n);
}
static int shape_h(S *x) { if (!x) return 0; int a = shape_h(x->l), b = shape_h(x->r); return 1 + (a > b ? a : b); }
static int hgt_iter(S *x)                       /* height via explicit stack, for chains of 10^4 nodes */
{
    if (!x) return 0; int cap = 1 << 16, top = 0, best = 0; S **st = malloc(cap * sizeof *st); int *d = malloc(cap * sizeof *d);
    st[top] = x; d[top++] = 1;
    while (top) { S *y = st[--top]; int dd = d[top]; if (dd > best) best = dd; if (y->l) { st[top] = y->l; d[top++] = dd + 1; } if (y->r) {
        st[top] = y->r; d[top++] = dd + 1; } }
    free(st); free(d); return best;
}
static void show(S *x) { if (!x) { printf("-"); return; } if (!x->l && !x->r) { printf("%d", x->key); return; } printf("(%d ", x->key);
    show(x->l); printf(" "); show(x->r); printf(")"); }
static void sfree(S *x) { if (x) { sfree(x->l); sfree(x->r); free(x); } }
/* hand-built tree from an S-expression-like builder */
static S *mk(int k, S *l, S *r) { S *n = calloc(1, sizeof *n); n->key = k; n->l = l; n->r = r; if (l) l->p = n; if (r) r->p = n; return n; }

/* AVL for comparison */
typedef struct A { int key, h; struct A *l, *r; } A;
static int ht(A *n) { return n ? n->h : 0; } static int mx(int a, int b) { return a > b ? a : b; }
static void fx(A *n) { n->h = 1 + mx(ht(n->l), ht(n->r)); }
static A *ar(A *y) { A *x = y->l; y->l = x->r; x->r = y; fx(y); fx(x); return x; }
static A *al(A *x) { A *y = x->r; x->r = y->l; y->l = x; fx(x); fx(y); return y; }
static A *avl(A *n, int k)
{
    if (!n) { n = calloc(1, sizeof *n); n->key = k; n->h = 1; return n; }
    if (k < n->key) n->l = avl(n->l, k); else if (k > n->key) n->r = avl(n->r, k); else return n;
    fx(n); int b = ht(n->l) - ht(n->r);
    if (b > 1 && k < n->l->key) return ar(n);
    if (b < -1 && k > n->r->key) return al(n);
    if (b > 1) { n->l = al(n->l); return ar(n); }
    if (b < -1) { n->r = ar(n->r); return al(n); }
    return n;
}
static int avl_visits(A *x, int k) { int v = 0; while (x) { v++; if (k == x->key) return v; x = k < x->key ? x->l : x->r; } return v; }
static void afree(A *n) { if (n) { afree(n->l); afree(n->r); free(n); } }
static unsigned long long rs = 88172645463325252ULL;
static unsigned rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (unsigned)(rs >> 11); }
static void shuffle(int *a, int n) { for (int i = n - 1; i > 0; i--) { int j = rnd() % (i + 1), x = a[i]; a[i] = a[j]; a[j] = x; } }

int main(void)
{
    /* Task 50: a given BST (the paper does not print one, so this tree is stated here) */
    S *t = mk(6, mk(4, mk(2, mk(1, NULL, NULL), mk(3, NULL, NULL)), mk(5, NULL, NULL)),
                 mk(8, mk(7, NULL, NULL), mk(10, mk(9, NULL, NULL), mk(11, NULL, NULL))));
    root = t; TRACE = 1; int f;
    printf("[T50] given tree: "); show(root); printf("\n");
    printf(" access 3 (search path 6 -> 4 -> 2 -> 3):\n"); int v = access_key(3, &f); printf("  visited=%d found=%d tree: ", v, f);
        show(root); printf(" root=%d\n", root->key);
    printf(" access 9:\n"); v = access_key(9, &f); printf("   visited=%d found=%d tree: ", v, f); show(root); printf(" root=%d\n",
        root->key);
    sfree(root); root = NULL; TRACE = 0;

    /* Tasks 52, 57, 60: N=1000 products, 10000 accesses, 80% of accesses go to the 50 hot products (5%) */
    enum { N = 1000, Q = 10000, HOT = 50 };
    int keys[N]; for (int i = 0; i < N; i++) keys[i] = i + 1;
    int ord[N]; memcpy(ord, keys, sizeof keys); shuffle(ord, N);
    int hot[HOT]; for (int i = 0; i < HOT; i++) hot[i] = ord[i];         /* a random 5% of the keys are the popular ones */
    int ins[N]; memcpy(ins, keys, sizeof keys); shuffle(ins, N);
    double sums[2][2][2] = {{{0}}}; /* [workload][structure][all/last half] */
    long worst[2][2] = {{0}}; long rot_total[2] = {0}; int first10_root[10], first10_key[10], first10_h[10];
    for (int w = 0; w < 2; w++) {                     /* 0 = skewed 80/5, 1 = uniform */
        root = NULL; ROT = 0; A *a = NULL;
        for (int i = 0; i < N; i++) { insert_key(ins[i]); a = avl(a, ins[i]); }
        long rot_start = ROT;
        unsigned long long save = rs;                  /* same access sequence for both structures */
        int seq[Q];
        for (int i = 0; i < Q; i++) seq[i] = (w == 0) ? ((rnd() % 100 < 80) ? hot[rnd() % HOT] : keys[rnd() % N]) : keys[rnd() % N];
        rs = save; (void)rs;
        for (int i = 0; i < Q; i++) {
            int k = seq[i], ff; int sv = access_key(k, &ff); int av = avl_visits(a, k);
            sums[w][0][0] += sv; sums[w][1][0] += av; if (i >= Q / 2) { sums[w][0][1] += sv; sums[w][1][1] += av; }
            if (sv > worst[w][0]) worst[w][0] = sv; if (av > worst[w][1]) worst[w][1] = av;
            if (w == 0 && i < 10) { first10_key[i] = k; first10_root[i] = root->key; first10_h[i] = shape_h(root); }
        }
        rot_total[w] = ROT - rot_start;
        sfree(root); root = NULL; afree(a);
    }
    printf("\n[T52,T57,T58] N=%d products, %d accesses; skewed = 80%% of accesses go to %d popular products (5%%), uniform = every product equally likely\n", N, Q, HOT);
    printf(" %-18s %-14s %-14s %-14s %-14s\n", "workload", "splay avg", "AVL avg", "splay avg", "AVL avg");
    printf(" %-18s %-14s %-14s %-14s %-14s\n", "", "(all 10000)", "(all 10000)", "(last 5000)", "(last 5000)");
    printf(" %-18s %-14.2f %-14.2f %-14.2f %-14.2f   (average nodes visited on the search path)\n", "skewed 80/5", sums[0][0][0] / Q,
        sums[0][1][0] / Q, sums[0][0][1] / (Q / 2), sums[0][1][1] / (Q / 2));
    printf(" %-18s %-14.2f %-14.2f %-14.2f %-14.2f\n", "uniform", sums[1][0][0] / Q, sums[1][1][0] / Q, sums[1][0][1] / (Q / 2), sums[1][1][1]
        / (Q / 2));
    printf(" worst single access (nodes visited): skewed splay=%ld AVL=%ld ; uniform splay=%ld AVL=%ld\n", worst[0][0], worst[0][1],
        worst[1][0], worst[1][1]);
    printf(" splay rotations per access: skewed %.2f, uniform %.2f   (AVL: 0 rotations during searches)\n", (double)rot_total[0] / Q,
        (double)rot_total[1] / Q);
    printf(" log2(%d) = %.2f\n", N, log2(N));
    printf("\n[T60] root after each of the first ten accesses (skewed workload)\n");
    for (int i = 0; i < 10; i++) printf(" access #%2d key=%4d -> root=%4d height=%d\n", i + 1, first10_key[i], first10_root[i],
        first10_h[i]);

    /* Tasks 58,59: sorted insertion makes a chain, one access costs n; accessing keys in sorted order afterwards */
    enum { M = 10000 };
    root = NULL; ROT = 0; for (int i = 1; i <= M; i++) insert_key(i);
    int h0 = hgt_iter(root);
    printf("\n[T58,T59] n=%d keys inserted in sorted order (each insertion splays the new key): height=%d, root=%d\n", M, h0, root->key);
    long tot = 0; int mxv = 0, first = 0, second = 0, third = 0;
    for (int k = 1; k <= M; k++) { int v = access_key(k, &f); tot += v; if (v > mxv) mxv = v; if (k == 1) first = v; if (k == 2) second = v; if
        (k == 3) third = v; }
    printf("   accessing keys 1,2,...,%d in sorted order: first access visits %d nodes (the whole chain), 2nd=%d, 3rd=%d, worst=%d, total=%ld, average=%.2f per access\n", M, first, second, third, mxv, tot, (double)tot / M);
    printf("   total rotations for the %d accesses = %ld ; height afterwards = %d\n", M, ROT, hgt_iter(root));
    sfree(root); root = NULL;
    /* random-access baseline for the same tree size */
    printf("   one more reference: %d accesses to random keys in a tree built by random insertion: ", 10000);
    int *pm = malloc(M * sizeof(int)); for (int i = 0; i < M; i++) pm[i] = i + 1; shuffle(pm, M); for (int i = 0; i < M; i++)
        insert_key(pm[i]);
    long tr = 0; for (int i = 0; i < 10000; i++) { int v = access_key(1 + rnd() % M, &f); tr += v; } printf("average %.2f nodes visited\n",
        (double)tr / 10000);
    sfree(root); free(pm);
    return 0;
}
