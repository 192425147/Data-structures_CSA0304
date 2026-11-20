#include <stdio.h>
#include <stdlib.h>
typedef struct Trie {
    struct Trie *child[26];
    int words;                              /* number of words passing through here */
    int end;                                /* 1 if a word ends here */
} Trie;
static int NODES;
Trie *node(void) { NODES++; return calloc(1, sizeof(Trie)); }
void insert(Trie *t, const char *w)
{
    /* TODO 1 (completed) */
    for (; *w >= 'a' && *w <= 'z'; w++) {
        int c = *w - 'a';
        if (!t->child[c]) t->child[c] = node();
        t = t->child[c];
        t->words++;
    }
    t->end = 1;
}
int count_prefix(const Trie *t, const char *p)
{
    /* TODO 2 (completed) */
    for (; *p; p++) {
        t = t->child[*p - 'a'];
        if (!t) return 0;
    }
    return t->words;
}
void destroy(Trie *t)
{
    if (!t) return;
    for (int i = 0; i < 26; i++) destroy(t->child[i]);
    free(t);
}
int main(void)
{
    const char *contacts[] = {"anand", "anitha", "anu", "arun", "bala", "anusuya"};
    Trie *root = node();
    for (int i = 0; i < 6; i++) insert(root, contacts[i]);
    const char *typed[] = {"a", "an", "anu", "b", "z"};
    for (int i = 0; i < 5; i++)
        printf("%s:%d ", typed[i], count_prefix(root, typed[i]));
    printf("\n");
    printf("nodes = %d (including the root), sizeof(Trie) = %zu bytes, memory = %zu bytes\n", NODES, sizeof(Trie), NODES * sizeof(Trie));
    destroy(root);
    return 0;
}
