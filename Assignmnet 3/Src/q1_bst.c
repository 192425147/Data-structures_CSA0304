/* Q1: complete BST library + experiments (tasks 1-4, 9-12) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct B { int k; struct B *l, *r; } B;

B *mk(int k) { B *n = calloc(1, sizeof *n); n->k = k; return n; }
/* iterative insert (no recursion depth problem for a degenerate tree); returns 1 if inserted, 0 if duplicate */
int insert(B **root, int k)
{
    B **p = root;
    while (*p) {
        if (k == (*p)->k) return 0;
        p = k < (*p)->k ? &(*p)->l : &(*p)->r;
    }
    *p = mk(k);
    return 1;
}
int search(B *t, int k, long *cmp)
{
    long c = 0;
    while (t) { c++; if (k == t->k) { *cmp = c; return 1; } t = k < t->k ? t->l : t->r; }
    *cmp = c; return 0;
}
int height(B *t) { if (!t) return -1; int a = height(t->l), b = height(t->r); return 1 + (a > b ? a : b); }
int size(B *t) { return t ? 1 + size(t->l) + size(t->r) : 0; }
void inorder(B *t, void (*f)(int)) { if (t) { inorder(t->l, f); f(t->k); inorder(t->r, f); } }
void preorder(B *t, void (*f)(int)) { if (t) { f(t->k); preorder(t->l, f); preorder(t->r, f); } }
void postorder(B *t, void (*f)(int)) { if (t) { postorder(t->l, f); postorder(t->r, f); f(t->k); } }
void levelorder(B *t, void (*f)(int))
{
    if (!t) return;
    int cap = size(t), h = 0, tl = 0; B **q = malloc(cap * sizeof *q);
    q[tl++] = t;
    while (h < tl) { B *x = q[h++]; f(x->k); if (x->l) q[tl++] = x->l; if (x->r) q[tl++] = x->r; }
    free(q);
}
static void pr(int k) { printf("%d ", k); }
/* delete: three cases */
B *del(B *t, int k, int *kase)
{
    if (!t) { *kase = 0; return NULL; }
    if (k < t->k) { t->l = del(t->l, k, kase); return t; }
    if (k > t->k) { t->r = del(t->r, k, kase); return t; }
    if (!t->l && !t->r) { *kase = 1; free(t); return NULL; }                    /* case 1: leaf */
    if (!t->l || !t->r) { *kase = 2; B *c = t->l ? t->l : t->r; free(t); return c; } /* case 2: one child */
    B *s = t->r; while (s->l) s = s->l;                                          /* case 3: inorder successor */
    *kase = 3; t->k = s->k; int dummy; t->r = del(t->r, s->k, &dummy);
    return t;
}
void destroy(B *t) { if (t) { destroy(t->l); destroy(t->r); free(t); } }

/* verification: BST property, returns size or -1 if violated */
int check(B *t, long lo, long hi)
{
    if (!t) return 0;
    if (t->k <= lo || t->k >= hi) return -1;
    int a = check(t->l, lo, t->k), b = check(t->r, t->k, hi);
    return (a < 0 || b < 0) ? -1 : a + b + 1;
}
static int buf[100000], nb;
static void collect(int k) { buf[nb++] = k; }
void show(B *t)       /* S-expression for the figure drawer */
{
    if (!t) { printf("-"); return; }
    if (!t->l && !t->r) { printf("%d", t->k); return; }
    printf("(%d ", t->k); show(t->l); printf(" "); show(t->r); printf(")");
}
/* file save/rebuild */
static FILE *fo; static void wr(int k) { fprintf(fo, "%d\n", k); }

int main(void)
{
    /* ---- Tasks 1-2: the sample tree and all four traversals ---- */
    int a[] = {50, 30, 70, 20, 40, 60, 80, 65};
    B *t = NULL; for (int i = 0; i < 8; i++) insert(&t, a[i]);
    printf("[T1] sample tree keys 50 30 70 20 40 60 80 65\n tree: "); show(t); printf("\n");
    printf(" preorder : "); preorder(t, pr); printf("\n inorder    : "); inorder(t, pr);
    printf("\n postorder : "); postorder(t, pr); printf("\n levelorder: "); levelorder(t, pr);
    printf("\n height = %d, size = %d\n", height(t), size(t));
    long c; search(t, 65, &c); printf(" search 65 -> %ld comparisons; ", c);
    int f = search(t, 66, &c); printf("search 66 -> found=%d after %ld comparisons\n", f, c);

    /* ---- Task 10: delete leaf, one-child node, two-child node, verify after each ---- */
    printf("\n[T10] deletions on the sample tree (verify = BST property + size + inorder sorted)\n");
    int dels[] = {20, 30, 50}, kase;
    for (int i = 0; i < 3; i++) {
        t = del(t, dels[i], &kase);
        nb = 0; inorder(t, collect); int sorted = 1; for (int j = 1; j < nb; j++) if (buf[j - 1] >= buf[j]) sorted = 0;
        printf(" delete %d (case %d): tree ", dels[i], kase); show(t);
        printf(" size=%d check=%d inorder_sorted=%d\n", size(t), check(t, -(1L << 40), 1L << 40), sorted);
    }
    destroy(t);

    /* ---- Tasks 3,4,9: 2000 keys random vs sorted ---- */
    enum { NK = 2000 };
    int keys[NK]; for (int i = 0; i < NK; i++) keys[i] = 1001 + i;           /* accession numbers 1001..3000 */
    B *rt = NULL, *st = NULL;
    for (int i = 0; i < NK; i++) insert(&st, keys[i]);
    int perm[NK]; memcpy(perm, keys, sizeof keys);
    srand(2024);
    for (int i = NK - 1; i > 0; i--) { int j = rand() % (i + 1), x = perm[i]; perm[i] = perm[j]; perm[j] = x; }
    for (int i = 0; i < NK; i++) insert(&rt, perm[i]);
    long sr = 0, ss = 0, wr_ = 0, ws = 0;
    for (int i = 0; i < NK; i++) {
        search(rt, keys[i], &c); sr += c; if (c > wr_) wr_ = c;
        search(st, keys[i], &c); ss += c; if (c > ws) ws = c;
    }
    printf("\n[T9] 2000 keys (1001..3000), successful searches of every key\n");
    printf(" random order: height=%d avg comparisons=%.2f max=%ld\n", height(rt), (double)sr / NK, wr_);
    printf(" sorted order: height=%d avg comparisons=%.2f max=%ld\n", height(st), (double)ss / NK, ws);
    printf(" log2(2000)=%.2f (balanced tree height would be %d)\n", 10.9658, 10);

    /* ---- Task 11: save with preorder, rebuild, compare ---- */
    fo = fopen("bst_saved.txt", "w"); preorder(rt, wr); fclose(fo);
    FILE *fi = fopen("bst_saved.txt", "r"); B *rb = NULL; int k, cnt = 0;
    while (fscanf(fi, "%d", &k) == 1) { insert(&rb, k); cnt++; } fclose(fi);
    int same_shape = 1;
    /* identical shape <=> identical preorder */
    nb = 0; preorder(rt, collect); int n1 = nb; static int p1[100000]; memcpy(p1, buf, n1 * sizeof(int));
    nb = 0; preorder(rb, collect);
    if (nb != n1 || memcmp(p1, buf, n1 * sizeof(int))) same_shape = 0;
    printf("\n[T11] saved %d keys by PREORDER to bst_saved.txt, rebuilt by inserting in file order: height %d vs %d, identical shape=%d\n",
           cnt, height(rt), height(rb), same_shape);
    /* the same with INORDER shows why it is the wrong choice */
    fo = fopen("bst_inorder.txt", "w"); inorder(rt, wr); fclose(fo);
    fi = fopen("bst_inorder.txt", "r"); B *ri = NULL; while (fscanf(fi, "%d", &k) == 1) insert(&ri, k); fclose(fi);
    printf("      same keys saved by INORDER and rebuilt: height %d (degenerate chain)\n", height(ri));
    destroy(rb); destroy(ri); destroy(rt); destroy(st);

    /* ---- Task 12: empty tree and duplicates ---- */
    printf("\n[T12] empty tree and duplicate keys\n");
    B *e = NULL;
    printf(" empty tree: height=%d size=%d ", height(e), size(e));
    int fe = search(e, 5, &c); printf("search(5) found=%d comparisons=%ld ", fe, c);
    e = del(e, 5, &kase); printf("delete(5) case=%d result=%s ", kase, e ? "non-empty" : "NULL");
    printf("inorder: ["); inorder(e, pr); printf("]\n");
    int ins1 = insert(&e, 7), ins2 = insert(&e, 7), ins3 = insert(&e, 3), ins4 = insert(&e, 3);
    printf(" insert 7,7,3,3 -> returned %d %d %d %d; size=%d (duplicates ignored); inorder: ", ins1, ins2, ins3, ins4, size(e));
    inorder(e, pr); printf("\n");
    e = del(e, 7, &kase); printf(" delete root 7 (case %d) -> ", kase); show(e); printf("\n");
    e = del(e, 3, &kase); printf(" delete last key 3 (case %d) -> tree empty=%d\n", kase, e == NULL);
    return 0;
}
