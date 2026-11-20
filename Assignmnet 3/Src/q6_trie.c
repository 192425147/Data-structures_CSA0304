/* Q6: full trie (insert, search, prefix count, autocomplete, delete) and experiments on synthetic contact names */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef struct T { struct T *ch[26]; int words, end; } T;
static long NODES;
T *nn(void) { NODES++; return calloc(1, sizeof(T)); }
void ins(T *t, const char *w) { for (const char *p = w; *p; p++) { int c = *p - 'a'; if (!t->ch[c]) t->ch[c] = nn(); t = t->ch[c]; t->words++;
    } t->end = 1; }
T *walk(T *t, const char *p) { for (; *p; p++) { t = t->ch[*p - 'a']; if (!t) return NULL; } return t; }
int search(T *t, const char *w) { T *x = walk(t, w); return x && x->end; }
int count_prefix(T *t, const char *p)
{
    if (*p == 0) { int s = t->end; for (int c = 0; c < 26; c++) if (t->ch[c]) s += t->ch[c]->words; return s; }   /* empty prefix: the root
        keeps no count of its own */
    T *x = walk(t, p); return x ? x->words : 0;
}
/* autocomplete: depth-first walk visiting children a..z gives alphabetical order */
static char buf[64]; static int out_n, out_max; static char (*out)[64];
void dfs(T *x, int len) { if (x->end && out_n < out_max) { buf[len] = 0; strcpy(out[out_n++], buf); } for (int c = 0; c < 26; c++) if
    (x->ch[c]) { buf[len] = 'a' + c; dfs(x->ch[c], len + 1); } }
int autocomplete(T *t, const char *p, char (*o)[64], int max) { T *x = walk(t, p); out_n = 0; out_max = max; out = o; if (!x) return 0;
    strcpy(buf, p); dfs(x, (int)strlen(p)); return out_n; }
/* delete: unmark the word, decrement the counts along the path and free nodes that no longer lead to any word */
int del(T *t, const char *w)
{
    if (!search(t, w)) return 0;
    T *x = t; for (const char *p = w; *p; p++) { int c = *p - 'a'; T *nx = x->ch[c]; nx->words--; if (nx->words == 0) { /* free the whole
        now-useless chain */ x->ch[c] = NULL; T *y = nx; while (y) { T *nxt = NULL; for (int i = 0; i < 26; i++) if (y->ch[i]) nxt = y->ch[i];
        free(y); NODES--; y = nxt; } return 1; } x = nx; }
    x->end = 0; return 1;
}
void destroy(T *t) { if (!t) return; for (int i = 0; i < 26; i++) destroy(t->ch[i]); free(t); }
static int cmps(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }
static double now(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return ts.tv_sec + ts.tv_nsec * 1e-9; }
static unsigned long long rs = 88172645463325252ULL;
static unsigned rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (unsigned)(rs >> 11); }
/* synthetic names: consonant-vowel syllables, 2..4 syllables, so many names share prefixes like real contact lists */
static char *gen(void) { static const char *C = "bcdfghjklmnprstvyz", *V = "aeiou"; char b[32]; int n = 0, syl = 2 + rnd() % 3;
    for (int i = 0; i < syl; i++) { b[n++] = C[rnd() % 18]; b[n++] = V[rnd() % 5]; if (rnd() % 3 == 0) b[n++] = C[rnd() % 18]; } b[n] = 0;
        return strdup(b); }
/* sorted array prefix query: lower_bound / upper_bound by two binary searches */
static int lower(char **a, int n, const char *p) { int lo = 0, hi = n, m = (int)strlen(p); while (lo < hi) { int mid = (lo + hi) / 2; if
    (strncmp(a[mid], p, m) < 0) lo = mid + 1; else hi = mid; } return lo; }
static int upper(char **a, int n, const char *p) { int lo = 0, hi = n, m = (int)strlen(p); while (lo < hi) { int mid = (lo + hi) / 2; if
    (strncmp(a[mid], p, m) <= 0) lo = mid + 1; else hi = mid; } return lo; }

int main(void)
{
    /* Tasks 61-63, 71, 72 on the six contacts */
    const char *c6[] = {"anand", "anitha", "anu", "arun", "bala", "anusuya"};
    T *r = nn(); for (int i = 0; i < 6; i++) ins(r, c6[i]);
    printf("[T61] six contacts: nodes=%ld (incl. root) memory=%zu bytes (sizeof node=%zu)\n", NODES, NODES * sizeof(T), sizeof(T));
    printf("      search: anu=%d an=%d (a prefix but not a stored word) anuj=%d ; count_prefix: a=%d an=%d anu=%d b=%d z=%d empty-prefix=%d\n",
           search(r, "anu"), search(r, "an"), search(r, "anuj"), count_prefix(r, "a"), count_prefix(r, "an"), count_prefix(r, "anu"),
               count_prefix(r, "b"), count_prefix(r, "z"), count_prefix(r, ""));
    static char o[100][64]; const char *pf[] = {"an", "anu", "a", "anusuya", "q", ""};
    printf("[T62,T71,T72] autocomplete lists (alphabetical):\n");
    for (int i = 0; i < 6; i++) { int n = autocomplete(r, pf[i], o, 100); printf("   prefix \"%s\" -> %d:", pf[i], n); for (int j = 0; j < n;
        j++) printf(" %s", o[j]); printf("\n"); }
    printf("[T63] delete (each step evaluated in order):\n");
    int d1 = del(r, "anu");
    printf("   delete anu -> %d : nodes=%ld, search(anu)=%d, search(anusuya)=%d, count(anu)=%d, count(an)=%d\n", d1, NODES, search(r, "anu"),
        search(r, "anusuya"), count_prefix(r, "anu"), count_prefix(r, "an"));
    long before = NODES; int d2 = del(r, "bala");
    printf("   delete bala -> %d : nodes %ld -> %ld (the chain b-a-l-a is freed), count(b)=%d, search(bala)=%d\n", d2, before, NODES,
        count_prefix(r, "b"), search(r, "bala"));
    int d3 = del(r, "zzz");
    printf("   delete missing 'zzz' -> %d (nothing changes)\n", d3);
    destroy(r); NODES = 0;

    /* Tasks 64, 69, 70: 500, 5000, 50000 synthetic names; time per prefix search; memory vs sorted array */
    int sizes[] = {500, 5000, 50000};
    printf("\n[T64,T69,T70] synthetic unique names (%s); prefix queries use the first 3 letters of random existing names\n", "2-4 consonant-vowel syllables");
    printf(" %7s %9s %11s %12s %12s %14s %14s %12s\n", "names", "trie nodes", "trie bytes", "array bytes", "ratio", "trie ns/query", "array ns/query", "avg matches");
    for (int s = 0; s < 3; s++) {
        int n = sizes[s]; char **a = malloc(n * 2 * sizeof *a); int m = 0;
        while (m < n) { a[m++] = gen(); }
        qsort(a, m, sizeof *a, cmps); int u = 0; for (int i = 0; i < m; i++) { if (u && !strcmp(a[u - 1], a[i])) free(a[i]); else a[u++] =
            a[i]; }
        while (u < n) { char *w = gen(); int dup = 0; for (int i = 0; i < u && !dup; i++) dup = !strcmp(a[i], w); if (dup) { free(w); continue;
            } a[u++] = w; qsort(a, u, sizeof *a, cmps); }
        n = u; NODES = 0; T *t = nn(); for (int i = 0; i < n; i++) ins(t, a[i]);
        long arr_bytes = 0; for (int i = 0; i < n; i++) arr_bytes += strlen(a[i]) + 1 + sizeof(char *);       /* string + pointer */
        int Q = 200000; char (*q)[4] = malloc(Q * 4);
        for (int i = 0; i < Q; i++) { const char *w = a[rnd() % n]; int L = strlen(w) < 3 ? strlen(w) : 3; memcpy(q[i], w, L); q[i][L] = 0; }
        volatile long sink = 0; long tot = 0;
        double t0 = now(); for (int i = 0; i < Q; i++) { int c = count_prefix(t, q[i]); sink += c; tot += c; } double t1 = now();
        double t2 = now(); for (int i = 0; i < Q; i++) { int c = upper(a, n, q[i]) - lower(a, n, q[i]); sink += c; } double t3 = now();
        int agree = 1; for (int i = 0; i < 20000; i++) if (count_prefix(t, q[i]) != upper(a, n, q[i]) - lower(a, n, q[i])) agree = 0;
        printf(" %7d %9ld %11zu %12ld %10.1fx %14.1f %14.1f %12.1f agree=%d\n", n, NODES, NODES * sizeof(T), arr_bytes, (double)(NODES *
            sizeof(T)) / arr_bytes, (t1 - t0) / Q * 1e9, (t3 - t2) / Q * 1e9, (double)tot / Q, agree);
        if (s == 2) { /* radix-style estimate: nodes with exactly one child and not a word end can be merged into their parent edge */
            long chain = 0, stack_n = 0; T **st = malloc(NODES * sizeof *st); st[stack_n++] = t;
            while (stack_n) { T *x = st[--stack_n]; int k = 0; for (int c = 0; c < 26; c++) if (x->ch[c]) { k++; st[stack_n++] = x->ch[c]; } if
                (x != t && k == 1 && !x->end) chain++; }
            printf("      radix estimate for %d names: %ld of %ld trie nodes have one child and no word end (mergeable) -> about %ld nodes remain (%.0f%% fewer)\n", n, chain, NODES, NODES - chain, 100.0 * chain / NODES);
            free(st);
        }
        sink += 0; destroy(t); for (int i = 0; i < n; i++) free(a[i]); free(a); free(q);
    }
    printf(" (array bytes = every name string + its 8-byte pointer; one 26-pointer trie node = %zu bytes)\n", sizeof(T));
    return 0;
}
