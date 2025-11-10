### Task Description

# Computational verification of a combinatorial hypothesis

For a given multiset of natural numbers $A$, denote $\sum A = \sum_{a \in A} a$.  
For example, if $A = \{1,2,2,2,10,10\}$ then $\sum A = 27$.  
For two multisets we write $A \supseteq B$ if each element occurs in $A$ at least as many times as in $B$.

## Definitions

**Definition.** A multiset $A$ is called *d-limited* for a natural number $d$ if it is finite and all its elements belong to $\{1, \dots, d\}$ (repetitions allowed).

**Definition.** A pair of d-limited multisets $A, B$ is called *disputed-free* if for all $A' \subseteq A$ and $B' \subseteq B$,
$$
\sum A' = \sum B' \iff A' = B' = \emptyset \text{ or } (A' = A \text{ and } B' = B).
$$
In other words: $\sum A = \sum B$, but the sums of any other non-empty subsets of $A$ and $B$ must differ.

## Problem

For a fixed $d \ge 3$ (smaller values of $d$ are not considered) and multisets $A_0, B_0$, we want to find d-limited disputed-free multisets $A \supseteq A_0$ and $B \supseteq B_0$ that maximize $\sum A$ (equivalently $\sum B$).  
Denote this value by $\alpha(d, A_0, B_0)$.  
Set $\alpha(d, A_0, B_0) = 0$ if $A_0$ and $B_0$ are not d-limited or have no d-limited disputed-free supersets.

### Examples
- $\alpha(d, \emptyset, \emptyset) \ge d(d-1)$.  
  Proof sketch: take $A = \{d,\dots,d\}$ (d−1 copies) and $B = \{d-1,\dots,d-1\}$ (d copies). Then $\sum A = d(d-1) = \sum B$.

- $\alpha(d, \emptyset, \{1\}) \ge (d-1)^2$.  
  Proof sketch: take $A = \{1,d,\dots,d\}$ (d−2 copies of $d$) and $B = \{d-1,\dots,d-1\}$ (d−1 copies). Then $\sum A = 1 + d(d-2) = (d-1)^2 = \sum B$.

These examples are actually optimal: $\alpha(d, \emptyset, \emptyset) = d(d-1)$ and $\alpha(d, \emptyset, \{1\}) = (d-1)^2$.

## Recursive backtracking approach

For a multiset $A$ define
$$
A_{\Sigma} = \{\sum A' : A' \subseteq A\}
$$
— the set of all possible subset sums of $A$ (considered as a set, multiplicities ignored).

A reference recursive procedure is:

**Solve(d, A, B):**
1. If $\sum A > \sum B$, swap $A$ and $B$.
2. Let $S = A_{\Sigma} \cap B_{\Sigma}$.

3. If $\sum A = \sum B$:
   - If $S = \{0, \sum A\}$ return $\sum A$.
   - Otherwise return $0$.

4. Else, if $S = \{0\}$:
   - Return $\max_{x \in \{\text{lastA},\dots,d\} \setminus B_{\Sigma}} \text{Solve}(d, A \cup \{x\}, B)$.

5. Otherwise (if $S$ contains other values) return $0$.

Here `lastA` denotes the last element added to $A$; for $A = A_0$ assume `lastA = 1` (recursion adds elements to $A_0$ non-decreasingly).

To avoid recomputing $A_{\Sigma}$ and $B_{\Sigma}$ from scratch, they are passed along. Adding an element $x$ to $A$ updates
$$
A_{\Sigma} \gets A_{\Sigma} \cup (A_{\Sigma} + x),
$$
where $A_{\Sigma} + x$ is the set obtained by adding $x$ to every element of $A_{\Sigma}$. Subset-sum sets are represented efficiently as bitsets.

## Task (programming)

Write **two alternative implementations** that compute $\alpha(d, A_0, B_0)$:

1. **Non-recursive**, single-threaded.
2. **Parallel**, multi-threaded, achieving the best possible scalability.

Also provide a **report** showing scalability of the parallel version on selected tests with varying thread counts.

**Important constraints:**
- Do **not** change the provided multiset/bitset operations — the program must perform exactly the same multiset operations as the reference implementation (order may differ in the parallel version).
- Parallelism must use `pthreads` (no processes).
- Use only standard and provided libraries.
- Compile with `gcc` (≥ 12.2) using `-std=gnu17 -march=native -O3 -pthread`.
- Memory: ≤ 128 MiB per thread (including main thread).
- Program must print only the solution to `stdout`. `stderr` may be used for logs but may affect performance.
- System call failures may terminate the program with `exit(1)`.

### Input / Output format

**Input (stdin):** three lines.

1. First line: four numbers `t d n m` — number of helper threads, parameter `d`, number of forced elements in `A0` and `B0`.
2. Second and third lines: the elements of `A0` (n numbers) and `B0` (m numbers) in the range $1..\!d$.

Example input:

8 10 0 1
1



**Output:** maximum sum $\sum A$, followed by multisets $A$ and $B$ in the format:
- First line: $\sum A$.
- Second and third lines: description of multisets $A$ and $B$.
  - For multiplicity $1$ write the element alone.
  - For multiplicity $k>1$ write `kx` followed by the element (for example `3x5`).
  - Elements listed in ascending order.

If no solution exists, output:

0

Example output (valid for the above input):

81
9x9
1 8x10


**Requirements**  

- 1 ≤ t ≤ 64, 3 ≤ d ≤ 50, 0 ≤ n ≤ 100, 0 ≤ m ≤ 100.  
- α(d, A0, B0) ≤ d(d−1) for all inputs.  
- Solution must traverse all recursion branches, matching the reference version.  
- Do **not** implement your own bitsets; only use the provided library.  
- Parallelism must use pthreads, no processes.  
- Only standard and provided libraries may be used.  
- Compilation: gcc ≥ 12.2 with `-std=gnu17 -march=native -O3 -pthread`.  
- Memory: ≤ 128 MiB per thread including main thread.  
- Program must print only the solution to stdout; stderr output is allowed but may reduce performance.  
- System call errors (e.g., memory allocation) may terminate the program with `exit(1)`.

**Report**  

- PDF format.  
- Show scalability of parallel solution for inputs: d ∈ {5,10,15,20,25,30,32,34}, A0 = ∅, B0 = {1}, number of threads = 1, 2, 4, 8, … up to 64.  
- If single-thread execution exceeds 1 minute, terminate and mark scalability as −1.  
- Measurement should be on a machine where reference implementation completes within 1 minute for input `1 34 1 0 1`.  

**Deliverables**  
- Non-recursive version: `nonrecursive/nonrecursive` executable.  
- Parallel version: `parallel/parallel` executable.  
- Report: `report.pdf`.  
- Optional additional files allowed if compilation works with:

unzip ab12345.zip
cmake -S ab12345/ -B build/ -DCMAKE_BUILD_TYPE=Release
cd build/
make
echo -n -e '1 3 1 0\n1\n\n' | ./nonrecursive/nonrecursive
echo -n -e '1 3 1 0\n1\n\n' | ./parallel/parallel


- Folder `common` must not be modified; CMakeLists.txt in root will be restored.  

- You may modify CMakeLists.txt in subfolders, `.clang-tidy`, and `.clang-format`.


