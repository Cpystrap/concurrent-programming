### Task Description

In the solutions from the previous laboratory tasks, we had one thread computing matrix elements for each of its columns.

For a matrix with a large number of columns, the cost of creating and managing threads in such a program would be significant.

Task  
Using the template in the file `MatrixRowSumsExecutors_skeleton.java`, write a new version of the program in which the elements of the matrix are computed by a **four-thread pool**.  
The computation of each matrix element should be a **separate task** submitted to the pool.  
Summation may be performed in the main thread.