### Task Description

The file `VectorStream_skeleton.java` shows an example of a **sequential** computation of a stream of vectors.  
Each element of the next vector is computed by some arbitrary function based on its index and the sum of elements of the previous vector (or zero for the first vector).

Implement the **same computation in parallel**, using **one thread per vector position** (do **not** create threads `STREAM_LENGTH` times). Threads should be synchronized using **semaphores, barriers, or latches** — consider which of these mechanisms is most appropriate for the task.

Requirements and constraints:

- The output must be printed **in the same order** as in the sequential version.
- Assume each call to `vectorDefinition.applyAsInt()` is **expensive** and should be executed in parallel.
- `STREAM_LENGTH` (the number of vectors) may be **very large** — do **not** allocate Ω(STREAM_LENGTH) memory; use **O(VECTOR_LENGTH)** memory instead.
- Ensure that **no two threads ever try to write to standard output at the same time**.