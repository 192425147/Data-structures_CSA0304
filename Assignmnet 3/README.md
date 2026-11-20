# Data Structures – Assignment 3 (Trees)

**Student:** D.Hasini  **Register No.:** 192425147

C programs for Assignment 3 (BST, AVL, B-tree, red-black, splay, trie and the dictionary design).
Every program was compiled with gcc and run; the output of each run is saved in `outputs/`.

## Folder layout

| Folder | Content |
|---|---|
| `src/` | The 12 C source files |
| `outputs/` | Output of each program (`<name>.txt`) |
| `data/` | Files written by `q1_bst.c` (Task 11: saved tree and its inorder list) |
| `run_all.sh` | Compiles and runs every program |

## Programs

| File | Question / tasks | What it does |
|---|---|---|
| `q1_pred1.c` | Q1 – predict the output (1) | Array-stored binary tree: preorder, inorder, postorder, level order |
| `q1_pred2.c` | Q1 – predict the output (2) | Second given program of the paper |
| `q1_complete.c` | Q1 – Complete the Code | Height and balance check with TODO 1 and TODO 2 completed |
| `q1_bst.c` | Q1, tasks 1–12 | BST insert, search, delete (3 cases), traversals, height, random vs sorted input, comparisons |
| `q2_complete.c` | Q2 – Complete the Code | AVL rotations completed |
| `q2_avl.c` | Q2, tasks 13–24 | AVL insertion with rotation counting; 100,000-key comparison with a plain BST |
| `q3_btree.c` | Q3, tasks 25–36 | B-tree insertion, search, disk-read counting, 5,000,000-key simulation |
| `q4_rb.c` | Q4, tasks 37–48 | Red-black tree with property checker and cached leftmost node; AVL with deletion for comparison |
| `q5_splay.c` | Q5, tasks 49–60 | Splay tree, 80/20 workload, sorted-order tests, AVL for comparison |
| `q6_complete.c` | Q6 – Complete the Code | Trie with both TODOs completed |
| `q6_trie.c` | Q6, tasks 61–72 | Trie with prefix count, autocomplete, delete, timing and memory |
| `q7_dict.c` | Q7, tasks 73–84 | Dictionary: compact trie, spell suggestions, range listing, verification and measurements |

## Compile and run

```bash
# small programs (with sanitizers)
gcc -Wall -Wextra -g -fsanitize=address,undefined src/q1_pred1.c -o q1_pred1 && ./q1_pred1

# experiment programs
gcc -O2 -Wall -Wextra src/q1_bst.c -o q1_bst -lm && ./q1_bst
```

Or run everything: `bash run_all.sh` (the big programs `q2_avl`, `q3_btree` and `q7_dict` take 15–30 seconds).

## Notes

* Timings (ns, ms) in the outputs depend on the computer; counts, heights, rotations and trees are the same on every run.
* Tested with gcc 13 on Linux.
