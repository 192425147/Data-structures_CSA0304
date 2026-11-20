/* Q4: red-black tree (CLRS) with insertion, deletion, property checker, leftmost cache; AVL tree with deletion for comparison */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
enum { RED, BLACK };
typedef struct RB { int key, color; struct RB *l, *r, *p; } RB;
static RB NILN = {0, BLACK, &NILN, &NILN, &NILN}, *NIL = &NILN;
static RB *root, *leftmost;                  /* leftmost = cached smallest node */
static long RB_ROT, RB_REC;                  /* rotations and recolourings (colour flips) */
static int VERB;
void setc(RB *x, int c) { if (x != NIL && x->color != c) { x->color = c; RB_REC++; } }
void lrot(RB *x) { RB *y = x->r; x->r = y->l; if (y->l != NIL) y->l->p = x; y->p = x->p;
    if (x->p == NIL) root = y; else if (x == x->p->l) x->p->l = y; else x->p->r = y; y->l = x; x->p = y; RB_ROT++; }
void rrot(RB *y) { RB *x = y->l; y->l = x->r; if (x->r != NIL) x->r->p = y; x->p = y->p;
    if (y->p == NIL) root = x; else if (y == y->p->l) y->p->l = x; else y->p->r = x; x->r = y; y->p = x; RB_ROT++; }
void rb_insert(int key)
{
    RB *z = calloc(1, sizeof *z); z->key = key; z->l = z->r = z->p = NIL; z->color = RED;
    RB *y = NIL, *x = root;
    while (x != NIL) { y = x; x = key < x->key ? x->l : x->r; }
    z->p = y; if (y == NIL) root = z; else if (key < y->key) y->l = z; else y->r = z;
    if (leftmost == NULL || leftmost == NIL || key < leftmost->key) leftmost = z;      /* cache update */
    while (z->p->color == RED) {
        RB *g = z->p->p;
        if (z->p == g->l) {
            RB *u = g->r;
            if (u->color == RED) { if (VERB) printf("    case 1 (uncle %d red): recolour parent %d and uncle %d black, grandparent %d red\n",
                 u->key, z->p->key, u->key, g->key);
                 setc(z->p, BLACK); setc(u, BLACK); setc(g, RED); z = g; }
            else {
                 if (z == z->p->r) { z = z->p; if (VERB) printf("   case 2 (uncle black, zig-zag): rotate left at %d\n", z->key); lrot(z); }
                 if (VERB) printf("   case 3 (uncle black, straight): recolour %d black, %d red; rotate right at %d\n", z->p->key, z->p->p->key,
                     z->p->p->key);
                 setc(z->p, BLACK); setc(z->p->p, RED); rrot(z->p->p);
            }
        } else {
            RB *u = g->l;
            if (u->color == RED) { if (VERB) printf("    case 1 (uncle %d red): recolour parent %d and uncle %d black, grandparent %d red\n",
                 u->key, z->p->key, u->key, g->key);
                 setc(z->p, BLACK); setc(u, BLACK); setc(g, RED); z = g; }
            else {
                 if (z == z->p->l) { z = z->p; if (VERB) printf("   case 2 (uncle black, zig-zag): rotate right at %d\n", z->key); rrot(z); }
                 if (VERB) printf("   case 3 (uncle black, straight): recolour %d black, %d red; rotate left at %d\n", z->p->key, z->p->p->key,
                     z->p->p->key);
                 setc(z->p, BLACK); setc(z->p->p, RED); lrot(z->p->p);
            }
        }
    }
    if (root->color == RED) setc(root, BLACK);
}
void transplant(RB *u, RB *v) { if (u->p == NIL) root = v; else if (u == u->p->l) u->p->l = v; else u->p->r = v; v->p = u->p; }
RB *minimum(RB *x) { while (x->l != NIL) x = x->l; return x; }
void del_fixup(RB *x)
{
    while (x != root && x->color == BLACK) {
        if (x == x->p->l) {
            RB *w = x->p->r;
            if (w->color == RED) { setc(w, BLACK); setc(x->p, RED); lrot(x->p); w = x->p->r; }
            if (w->l->color == BLACK && w->r->color == BLACK) { setc(w, RED); x = x->p; }
            else {
                 if (w->r->color == BLACK) { setc(w->l, BLACK); setc(w, RED); rrot(w); w = x->p->r; }
                 setc(w, x->p->color); setc(x->p, BLACK); setc(w->r, BLACK); lrot(x->p); x = root;
            }
        } else {
            RB *w = x->p->l;
            if (w->color == RED) { setc(w, BLACK); setc(x->p, RED); rrot(x->p); w = x->p->l; }
            if (w->r->color == BLACK && w->l->color == BLACK) { setc(w, RED); x = x->p; }
            else {
                 if (w->l->color == BLACK) { setc(w->r, BLACK); setc(w, RED); lrot(w); w = x->p->l; }
                 setc(w, x->p->color); setc(x->p, BLACK); setc(w->l, BLACK); rrot(x->p); x = root;
            }
        }
    }
    setc(x, BLACK);
}
void rb_delete(int key)
{
    RB *z = root; while (z != NIL && z->key != key) z = key < z->key ? z->l : z->r;
    if (z == NIL) return;
    if (z == leftmost) leftmost = (z->r != NIL) ? minimum(z->r) : z->p;           /* successor of the leftmost = its right subtree's min or its
        parent */
    RB *y = z, *x; int yc = y->color;
    if (z->l == NIL) { x = z->r; transplant(z, z->r); }
    else if (z->r == NIL) { x = z->l; transplant(z, z->l); }
    else {
        y = minimum(z->r); yc = y->color; x = y->r;
        if (y->p == z) x->p = y; else { transplant(y, y->r); y->r = z->r; y->r->p = y; }
        transplant(z, y); y->l = z->l; y->l->p = y; y->color = z->color;
    }
    free(z);
    if (yc == BLACK) del_fixup(x);
    NILN.p = NILN.l = NILN.r = NIL; NILN.color = BLACK;
    if (root == NIL) leftmost = NIL;
}
/* checker: returns black height or -1; also checks BST order, five properties, parent pointers */
long RB_NODES;
int rb_check_node(RB *x, long lo, long hi)
{
    if (x == NIL) return 1;                                                                  /* property 3: NIL leaves are black */
    if (x->key <= lo || x->key >= hi) return -1;
    if (x->color == RED && (x->l->color == RED || x->r->color == RED)) return -1;            /* property 4 */
    if ((x->l != NIL && x->l->p != x) || (x->r != NIL && x->r->p != x)) return -1;
    int a = rb_check_node(x->l, lo, x->key), b = rb_check_node(x->r, x->key, hi);
    if (a < 0 || b < 0 || a != b) return -1;                                                 /* property 5 */
    RB_NODES++;
    return a + (x->color == BLACK);
}
int rb_check(void)
{
    RB_NODES = 0;
    if (root != NIL && root->color != BLACK) return 0;                                       /* property 2 */
    if (rb_check_node(root, -(1L << 40), 1L << 40) < 0) return 0;
    RB *m = root; if (m != NIL) while (m->l != NIL) m = m->l;
    if (root == NIL ? (leftmost != NIL && leftmost != NULL) : leftmost != m) return 0;       /* cached leftmost correct */
    return 1;
}
int rb_height(RB *x) { if (x == NIL) return 0; int a = rb_height(x->l), b = rb_height(x->r); return 1 + (a > b ? a : b); }
void rb_show(RB *x) { if (x == NIL) { printf("-"); return; } if (x->l == NIL && x->r == NIL) { printf("%d%c", x->key, x->color == RED ? 'R' :
    'B'); return; }
    printf("(%d%c ", x->key, x->color == RED ? 'R' : 'B'); rb_show(x->l); printf(" "); rb_show(x->r); printf(")"); }
void rb_free(RB *x) { if (x != NIL) { rb_free(x->l); rb_free(x->r); free(x); } }

/* ---------------- AVL with deletion ---------------- */
typedef struct Nd { int key, h; struct Nd *l, *r; } Nd;
static long AVL_ROT;
int ht(Nd *n) { return n ? n->h : 0; }
int mx(int a, int b) { return a > b ? a : b; }
void fx(Nd *n) { n->h = 1 + mx(ht(n->l), ht(n->r)); }
int bf(Nd *n) { return n ? ht(n->l) - ht(n->r) : 0; }
Nd *ar(Nd *y) { Nd *x = y->l; y->l = x->r; x->r = y; fx(y); fx(x); AVL_ROT++; return x; }
Nd *al(Nd *x) { Nd *y = x->r; x->r = y->l; y->l = x; fx(x); fx(y); AVL_ROT++; return y; }
Nd *rebalance(Nd *n)
{
    fx(n); int b = bf(n);
    if (b > 1) { if (bf(n->l) < 0) n->l = al(n->l); return ar(n); }
    if (b < -1) { if (bf(n->r) > 0) n->r = ar(n->r); return al(n); }
    return n;
}
Nd *avl_ins(Nd *n, int k) { if (!n) { n = calloc(1, sizeof *n); n->key = k; n->h = 1; return n; } if (k < n->key) n->l = avl_ins(n->l, k); else
    if (k > n->key) n->r = avl_ins(n->r, k); else return n; return rebalance(n); }
Nd *avl_del(Nd *n, int k)
{
    if (!n) return NULL;
    if (k < n->key) n->l = avl_del(n->l, k); else if (k > n->key) n->r = avl_del(n->r, k);
    else {
        if (!n->l || !n->r) { Nd *c = n->l ? n->l : n->r; free(n); return c; }
        Nd *s = n->r; while (s->l) s = s->l; n->key = s->key; n->r = avl_del(n->r, s->key);
    }
    return rebalance(n);
}
int avl_check(Nd *t, long lo, long hi)
{
    if (!t) return 0;
    if (t->key <= lo || t->key >= hi) return -1;
    int a = avl_check(t->l, lo, t->key), b = avl_check(t->r, t->key, hi); if (a < 0 || b < 0) return -1;
    int d = a > b ? a - b : b - a; if (d > 1 || t->h != 1 + mx(a, b)) return -1; return 1 + mx(a, b);
}
void avl_free(Nd *n) { if (n) { avl_free(n->l); avl_free(n->r); free(n); } }
static unsigned long long rs = 88172645463325252ULL;
static unsigned rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (unsigned)(rs >> 11); }
void shuffle(int *a, int n) { for (int i = n - 1; i > 0; i--) { int j = rnd() % (i + 1), x = a[i]; a[i] = a[j]; a[j] = x; } }

int main(void)
{
    /* Tasks 38,39,40: hand example traced by the implementation */
    root = leftmost = NIL; VERB = 1;
    int ks[] = {10, 20, 30, 15, 25, 5};
    printf("[T38,T39] red-black insertion of 10 20 30 15 25 5 (R = red, B = black)\n");
    for (int i = 0; i < 6; i++) {
        long r0 = RB_ROT, c0 = RB_REC;
        printf(" insert %d\n", ks[i]); rb_insert(ks[i]);
        printf("   -> tree: "); rb_show(root); printf("   [rotations this step=%ld, recolourings=%ld, properties OK=%d, leftmost cache=%d]\n",
            RB_ROT - r0, RB_REC - c0, rb_check(), leftmost->key);
    }
    printf(" [T40] smallest process = leftmost node = %d (cached pointer, O(1)); by walking left from the root: %d\n", leftmost->key,
        minimum(root)->key);
    rb_free(root); root = leftmost = NIL; VERB = 0;

    /* Tasks 45,46,47: 10000 random insertions then 10000 random deletions, checker after every operation */
    enum { N = 10000 };
    int *ins = malloc(N * sizeof(int)), *del = malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) ins[i] = del[i] = i + 1;
    shuffle(ins, N); shuffle(del, N);
    RB_ROT = RB_REC = 0; AVL_ROT = 0; Nd *a = NULL; root = leftmost = NIL;
    long rb_ri = 0, av_ri = 0, rb_rd = 0, av_rd = 0, rb_ci = 0, rb_cd = 0, bad = 0, checks = 0; int rb_max_i = 0, rb_max_d = 0, av_max_i = 0,
        av_max_d = 0, rbh_mid = 0, avh_mid = 0;
    for (int i = 0; i < N; i++) {
        long r0 = RB_ROT, c0 = RB_REC, a0 = AVL_ROT;
        rb_insert(ins[i]); a = avl_ins(a, ins[i]);
        long dr = RB_ROT - r0, da = AVL_ROT - a0; if (dr > rb_max_i) rb_max_i = dr; if (da > av_max_i) av_max_i = da;
        rb_ri += dr; rb_ci += RB_REC - c0; av_ri += da;
        checks++; if (!rb_check() || avl_check(a, -1, 1L << 40) < 0) bad++;
    }
    int rbh = rb_height(root), avh = ht(a);
    for (int i = 0; i < N; i++) {
        long r0 = RB_ROT, c0 = RB_REC, a0 = AVL_ROT;
        rb_delete(del[i]); a = avl_del(a, del[i]);
        long dr = RB_ROT - r0, da = AVL_ROT - a0; if (dr > rb_max_d) rb_max_d = dr; if (da > av_max_d) av_max_d = da;
        rb_rd += dr; rb_cd += RB_REC - c0; av_rd += da;
        checks++; if (!rb_check() || avl_check(a, -1, 1L << 40) < 0) bad++;
        if (i == N / 2 - 1) { rbh_mid = rb_height(root); avh_mid = ht(a); }
    }
    printf("\n[T45,T46,T47] %d random insertions then the same %d keys deleted in another random order\n", N, N);
    printf(" checker (5 properties + BST order + cached leftmost for RB; balance factors for AVL) run after each of %ld operations: %ld failures\n", checks, bad);
    printf(" %-28s %10s %10s\n", "", "red-black", "AVL");
    printf(" %-28s %10ld %10ld\n", "rotations, 10000 insertions", rb_ri, av_ri);
    printf(" %-28s %10ld %10ld\n", "rotations, 10000 deletions", rb_rd, av_rd);
    printf(" %-28s %10.3f %10.3f\n", "rotations per insertion", (double)rb_ri / N, (double)av_ri / N);
    printf(" %-28s %10.3f %10.3f\n", "rotations per deletion", (double)rb_rd / N, (double)av_rd / N);
    printf(" %-28s %10d %10d\n", "max rotations, one insertion", rb_max_i, av_max_i);
    printf(" %-28s %10d %10d\n", "max rotations, one deletion", rb_max_d, av_max_d);
    printf(" red-black recolourings (colour flips, no pointer change): insertions %ld, deletions %ld\n", rb_ci, rb_cd);
    printf(" %-28s %10d %10d\n", "height (levels) after inserts", rbh, avh);
    printf(" %-28s %10d %10d\n", "height after 5000 deletions", rbh_mid, avh_mid);
    printf(" bounds: red-black <= 2*log2(n+1) = %.2f ; AVL <= 1.44*log2(n+2) = %.2f ; minimum possible = %d\n", 2 * log2(N + 1.0), 1.4405 *
        log2(N + 2.0) - 0.3277, 14);
    avl_free(a); rb_free(root); root = leftmost = NIL;

    /* Task 48: increasing-order insertion */
    RB_ROT = 0; AVL_ROT = 0; a = NULL; int badinc = 0;
    for (int i = 1; i <= N; i++) { rb_insert(i); a = avl_ins(a, i); if (!rb_check() || avl_check(a, -1, 1L << 40) < 0) badinc++; }
    printf("\n[T48] keys inserted in increasing order 1..%d: red-black height=%d rotations=%ld ; AVL height=%d rotations=%ld ; checker failures=%d\n",
           N, rb_height(root), RB_ROT, ht(a), AVL_ROT, badinc);
    printf("      leftmost (smallest) after sorted insertion = %d; largest-key path length of red-black tree = %d\n", leftmost->key,
        rb_height(root));
    avl_free(a); rb_free(root); free(ins); free(del);
    return 0;
}
