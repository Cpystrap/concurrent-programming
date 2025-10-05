### Task Description

In the `Vector` class, whose objects represent vectors of a given length, implement the method  
`Vector sum(Vector other)` that computes the sum of this vector with another vector of the same length,  
and the method `int dot(Vector other)` that computes their dot product.

Both methods should perform the computation using **multiple threads**.  
Assign helper threads the task of adding or performing element-wise multiplication on **fragments of the vectors of length 10**.  
(In the case of `dot`, the aggregation of partial results may be done by the main thread.)

Use the `join()` method to wait for the helper threads to finish.  
Make sure to handle interruptions correctly.