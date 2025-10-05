package lab03.assignments;

import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.BrokenBarrierException;
import java.util.function.IntBinaryOperator;
import java.util.concurrent.atomic.AtomicInteger;

public class VectorStream {
    private static final int STREAM_LENGTH = 10;
    private static final int VECTOR_LENGTH = 100;

    private static final CyclicBarrier barrier = new CyclicBarrier(VECTOR_LENGTH);
    private static final AtomicInteger Sum = new AtomicInteger(0);
    // Global array to store results
    private static final int[] Vector = new int[VECTOR_LENGTH];

    /**
     * Function that defines how vectors are computed: the i-th element depends on
     * the previous sum and the index i.
     * The sum of elements in the previous vector is initially given as zero.
     */

    private final static IntBinaryOperator vectorDefinition = (previousSum, i) -> {
        int a = 2 * i + 1;
        return (previousSum / VECTOR_LENGTH + 1) * (a % 4 - 2) * a;
    };

    private static class VectorCounter implements Runnable {
        private final int cell;

        private VectorCounter(final int cell) {
            this.cell = cell;
        }

        @Override
        public void run() {
            for (int vectorNo = 0; vectorNo < STREAM_LENGTH; vectorNo++) {
                // Check for interruption before starting the vector calculation
                if (Thread.currentThread().isInterrupted()) {
                    System.out.println("Thread " + Thread.currentThread().getName() + " interrupted before starting vector " + vectorNo);
                    return; // Exit the run method
                }
                try {
                    // Wait for all threads to reach the barrier before accessing the
                    // shared sum
                    barrier.await();

                    // Get the current sum for this vector iteration
                    int tempSum = Sum.get();

                    // Check for interruption after barrier wait
                    if (Thread.currentThread().isInterrupted()) {
                        System.out.println("Thread " + Thread.currentThread().getName() + " interrupted after barrier for vector " + vectorNo);
                        return; // Exit the run method
                    }

                    // Calculate the new value based on the vector index
                    // Store the new value in the vector array
                    Vector[cell] = vectorDefinition.applyAsInt(tempSum, cell);

                    // Wait for all threads to finish calculations
                    barrier.await();

                    // Now update the sum only after all threads have calculated
                    // their values, only 1. thread should do it
                    if (cell == 0) {
                        int updatedSum = 0; // Reset for the updated sum
                        for (int value : Vector) {
                            updatedSum += value; // Aggregate results from all threads
                        }
                        // Update the shared sum to the newly computed sum
                        Sum.set(updatedSum);
                        // Print the current vector number and its total sum
                        System.out.println(vectorNo + " -> " + Sum.get());
                    }
                } catch (InterruptedException e) {
                    System.err.println(Thread.currentThread().getName() + " interrupted.");
                    Thread.currentThread().interrupt(); // Restore interrupted status
                    return;
                } catch (BrokenBarrierException e) {
                    System.err.println("Barrier broken: " + e.getMessage());
                    return;
                }
            }
        }
    }

    private static void computeVectorStreamSequentially() {
        int[] vector = new int[VECTOR_LENGTH];
        int sum = 0;
        for (int vectorNo = 0; vectorNo < STREAM_LENGTH; ++vectorNo) {
            for (int i = 0; i < VECTOR_LENGTH; ++i) {
                vector[i] = vectorDefinition.applyAsInt(sum, i);
            }
            sum = 0;
            for (int x : vector) {
                sum += x;
            }
            System.out.println(vectorNo + " -> " + sum);
        }
    }

    private static void computeVectorStreamInParallel() throws InterruptedException {
        Thread[] threads = new Thread[VECTOR_LENGTH];
        for (int i = 0; i < VECTOR_LENGTH; i++) {
            final int index = i;
            threads[i] = new Thread(new VectorCounter(index));
            threads[i].start();
        }

        // Wait for all threads to finish
        for (int i = 0; i < VECTOR_LENGTH; i++) {
            try {
                threads[i].join(); // Ensure main thread waits for all threads to complete
            } catch (InterruptedException e) {
                // delete all of its children before deleting main thread
                int deleteChildren = VECTOR_LENGTH;
                while (deleteChildren > 0) {
                    for (int j = deleteChildren - 1; j >= 0; j--) {
                        try {
                            threads[j].join();
                            deleteChildren--;
                        } catch (InterruptedException ex) {
                            j = deleteChildren - 1;
                        }
                    }
                }
                Thread.currentThread().interrupt(); // Restore interrupted status
                System.err.println("Main thread interrupted: " + e.getMessage());
            }
        }
    }

    public static void main(String[] args) {
        try {
            System.out.println("-- Sequentially --");
            computeVectorStreamSequentially();
            System.out.println("-- Parallel --");
            computeVectorStreamInParallel();
            System.out.println("-- End --");
        } catch (InterruptedException e) {
            System.err.println("Main interrupted.");
        }
    }
}
