### Task Description

**Computational verification of a combinatorial hypothesis**

For a given multiset of natural numbers \(A\), denote \(\sum A = \sum_{a \in A} a\).  
For example, if \(A = \{1,2,2,2,10,10\}\), then \(\sum A = 27\).  
For two multisets, we write \(A \supseteq B\) if each element occurs in \(A\) at least as many times as in \(B\).  

For the purposes of this task, we adopt the following definitions:

**Definition.** A multiset \(A\) is called *d-limited* for a natural number \(d\) if it is finite and all its elements belong to \(\{1, \dots, d\}\) (with repetitions allowed).

**Definition.** A pair of d-limited multisets \(A, B\) is called *disputed-free* if for all \(A' \subseteq A\) and \(B' \subseteq B\),  
\[
\sum A' = \sum B' \iff A' = B' = \emptyset \text{ or } (A' = A \text{ and } B' = B).
\]  
In other words, \(\sum A = \sum B\), but sums of any other non-empty subsets of \(A\) and \(B\) must differ.

**Problem.** For a fixed \(d \ge 3\) (smaller values of \(d\) are not considered) and multisets \(A_0, B_0\), we want to find d-limited disputed-free multisets \(A \supseteq A_0\) and \(B \supseteq B_0\) that maximize \(\sum A\) (equivalently, \(\sum B\)).  
Denote this value by \(\alpha(d, A_0, B_0)\).  
Set \(\alpha(d, A_0, B_0) = 0\) if \(A_0\) and \(B_0\) are not d-limited or have no d-limited disputed-free supersets.

**Examples.**  
- \(\alpha(d, \emptyset, \emptyset) \ge d(d-1)\).  
  Proof sketch: sets \(A = \{d, \dots, d\) (d−1 times)\} and \(B = \{d-1, \dots, d-1\) (d times)\} satisfy the conditions for \(\sum A = d(d-1) = \sum B\).

- \(\alpha(d, \emptyset, \{1\}) \ge (d-1)^2\).  
  Proof sketch: sets \(A = \{1, d, \dots, d\) (d−2 times)\} and \(B = \{d-1, \dots, d-1\) (d−1 times)\} satisfy the conditions for \(\sum A = 1 + d(d-2) = (d-1)^2 = \sum B\).

These examples are in fact optimal: \(\alpha(d, \emptyset, \emptyset) = d(d-1)\) and \(\alpha(d, \emptyset, \{1\}) = (d-1)^2\).

**Recursive backtracking approach**  

Values of \(\alpha(d, A_0, B_0)\) can be computed recursively by incrementally building multisets \(A \supseteq A_0\) and \(B \supseteq B_0\).  
Let \(A_\Sigma = \{\sum A' : A' \subseteq A\}\) denote the set of all possible sums obtainable from elements of \(A\) (as a set, multiplicities ignored).  

The recursion is:

Solve(d, A, B):
if sum(A) > sum(B):
    swap(A, B)
S = AΣ ∩ BΣ

if sum(A) = sum(B):
    if S = {0, sum(A)}:
        return sum(A)
    else:
        return 0
else if S = {0}:
    return max_{x in {lastA,...,d} \ BΣ} Solve(d, A ∪ {x}, B)
else:
    return 0


Here, `lastA` denotes the last element added to `A`; for `A = A0` assume `lastA = 1` (recursion adds elements to `A0` non-decreasingly).

To avoid recomputing sum sets \(A_\Sigma\) and \(B_\Sigma\) from scratch, they are passed along.  
Adding an element \(x\) to \(A\) updates \(A_\Sigma \gets A_\Sigma \cup (A_\Sigma + x)\), where \(A_\Sigma + x\) is the set obtained by adding \(x\) to every element of \(A_\Sigma\).  
Sum sets are efficiently represented using bitsets.

**Task**  

Write **two alternative implementations** of this computation:

1. **Non-recursive**, single-threaded.
2. **Parallel**, multi-threaded, achieving the best possible scalability.

Also provide a **report** showing scalability of the parallel version on selected tests with varying numbers of threads.

- The goal is **not** theoretical analysis or micro-optimizations of bit operations; the multiset operations are provided and **must not be changed**.
- The solution should perform exactly the same operations on multisets as the reference implementation (order can differ in the parallel version).

**Scalability coefficient:**  

Let \(t_s\) be the runtime of the reference version on a given input and \(t_n\) the runtime of the parallel version on \(n\) helper threads (assuming at least \(n\) physical cores).  
Scalability is \(t_s / t_n\), ideally \(n\) for perfectly scaling code.

**Suggested approach:**  

- Implement non-recursive version first (e.g., using a custom stack).  
- Then parallelize it.  
- Pay attention to memory allocation/deallocation in the non-recursive version.

**Parallelization challenge:**  

- It is difficult to predict which recursion branches are computationally heaviest.  
- Threads should mostly work on their own branch, occasionally sharing sub-branches to a common task pool.  
- Idle threads can pick up tasks from the shared pool.

**Input/Output format**  

Input: three lines on standard input.  

1. First line: four numbers `t d n m` — number of helper threads, parameter d, number of forced elements in A0 and B0.  
2. Second and third lines: the elements of A0 (n numbers) and B0 (m numbers) in the range 1..d.

Compute \(\alpha(d, A_0, B_0)\) using `t` helper threads.

Example input:

8 10 0 1
1


Output: maximum sum \(\sum A\), followed by multisets A and B in the format:

- First line: \(\sum A\)  
- Second and third lines: description of multisets A and B  
- For elements with multiplicity 1, write just the element; for multiplicity k > 1, write `kx` followed by element.  
- Elements are listed in ascending order.

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
