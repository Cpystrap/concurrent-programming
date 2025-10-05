### Task Description

In the solution of the task from the previous classes, the thread that computed an element of the vector, before moving to the next one, had to wait for the current one to be summed and for the sum to be printed. In today's task we similarly compute successive rows of a matrix, but without this restriction.

Assignment
Using the template from the file `MatrixRowSums_skeleton.java`, write a new concurrent program enabling:

- computing elements of successive rows without waiting for the previous ones to be summed,
- starting the summation of a row before all of its elements have been computed.

Ensure the efficiency of the solution in the case where the number of rows is very large.