package lab02.assignments;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Random;

@SuppressWarnings("unused")
public class Vector {
    private static final int SUM_CHUNK_LENGTH = 10;
    private static final int DOT_CHUNK_LENGTH = 10;

    private final int[] elements;

    public Vector(int length) {
        elements = new int[length];
    }

    public Vector(int[] elements) {
        this.elements = Arrays.copyOf(elements, elements.length);
    }

    private final Vector sumSequential(Vector other) {
        if (this.elements.length != other.elements.length) {
            throw new IllegalArgumentException("Vector lengths differ.");
        }
        Vector result = new Vector(this.elements.length);
        for (int i = 0; i < result.elements.length; ++i) {
            result.elements[i] = this.elements[i] + other.elements[i];
        }
        return result;
    }

    private final int dotSequential(Vector other) {
        if (this.elements.length != other.elements.length) {
            throw new IllegalArgumentException("Vector lengths differ.");
        }
        int result = 0;
        for (int i = 0; i < this.elements.length; ++i) {
            result += this.elements[i] * other.elements[i];
        }
        return result;
    }

    private static class SumHelper implements Runnable {
        private final Vector left;
        private final Vector right;
        private final Vector result;
        private final int begin;
        private final int end;

        public SumHelper(Vector left, Vector right, Vector result, int begin, int end) {
            this.left = left;
            this.right = right;
            this.result = result;
            this.begin = begin;
            this.end = end;
        }

        @Override
        public void run() {
            for (int i = begin; i < end; ++i) {
                // Checking whether the thread has been interrupted
                if (Thread.currentThread().isInterrupted()) {
                    System.err.println("Thread interrupted during sum operation");
                    return;
                }
                result.elements[i] = left.elements[i] + right.elements[i];
            }
        }
    }

    public Vector sum(Vector v) throws InterruptedException {
        if (elements.length != v.elements.length) {
            throw new IllegalArgumentException("Vector lengths differ.");
        }

        Vector result = new Vector(elements.length);
        ArrayList<Thread> threads = new ArrayList<>();
        // Divide the task into chunks of length SUM_CHUNK_LENGTH
        for (int i = 0; i < elements.length; i += SUM_CHUNK_LENGTH) {
            int begin = i;
            int end = Math.min(i + SUM_CHUNK_LENGTH, elements.length);

            // Create a thread for calculating part of the sum
            Thread thread = new Thread(new SumHelper(this, v, result, begin, end));
            threads.add(thread);
            thread.start();
        }

        // Waiting for threads to end
        for (Thread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();  // Transmission of interruption information
                System.err.println("Thread interrupted during sum operation");
                throw e;  // Abort method by throwing exception again
            }
        }

        return result;
    }

    private static class DotHelper implements Runnable {
        private final Vector left;
        private final Vector right;
        private final int begin;
        private final int end;
        private final int[] result;
        private final int resultPosition;

        public DotHelper(Vector left, Vector right, int begin, int end, int[] result, int resultPosition) {
            this.left = left;
            this.right = right;
            this.begin = begin;
            this.end = end;
            this.result = result;
            this.resultPosition = resultPosition;
        }

        @Override
        public void run() {
            int sum = 0;
            for (int i = begin; i < end; i++) {
                // Checking whether the thread has been interrupted
                if (Thread.currentThread().isInterrupted()) {
                    System.err.println("Thread interrupted during dot product operation");
                    return;
                }
                sum += left.elements[i] * right.elements[i];
            }
            result[resultPosition] = sum;
        }
    }

    public int dot(Vector v) throws InterruptedException {
        if (elements.length != v.elements.length) {
            throw new IllegalArgumentException("Vector lengths differ.");
        }

        // round up the number of pieces (chunks) needed
        int nThreads = (elements.length + DOT_CHUNK_LENGTH - 1) / DOT_CHUNK_LENGTH;
        int[] partialResults = new int[nThreads];
        ArrayList<Thread> threads = new ArrayList<>();
        // Creating threads to calculate scalar product fragments.
        for (int i = 0; i < elements.length; i += DOT_CHUNK_LENGTH) {
            int begin = i;
            int end = Math.min(i + DOT_CHUNK_LENGTH, elements.length);
            int threadIndex = i / DOT_CHUNK_LENGTH;

            // Create a thread for calculating the product fragment
            Thread thread = new Thread(new DotHelper(this, v, begin, end, partialResults, threadIndex));
            threads.add(thread);
            thread.start();
        }

        // Waiting for threads to end
        for (Thread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();  // Transmission of interruption information
                System.err.println("Thread interrupted during dot product operation");
                throw e;  // Abort method by throwing exception again
            }
        }

        // the main thread make aggregation of fragmented results
        int total = 0;
        for (int result : partialResults) {
            total += result;
        }

        return total;
    }

    // ----------------------- TESTS -----------------------

    @Override
    public String toString() {
        return Arrays.toString(elements);
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof Vector)) {
            return false;
        }
        Vector other = (Vector) obj;
        return Arrays.equals(this.elements, other.elements);
    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(this.elements);
    }

    private static final Random random = new Random(42);

    private static Vector generateRandomVector(int length) {
        int[] a = new int[length];
        for (int i = 0; i < length; ++i) {
            a[i] = random.nextInt(10);
        }
        return new Vector(a);
    }

    public static void main(String[] args) {
        try {
            for (int length : new int[] { 33, 30 }) {
                Vector a = generateRandomVector(length);
                System.out.println("A:        " + a);
                Vector b = generateRandomVector(length);
                System.out.println("B:        " + b);

                Vector c = a.sum(b);
                Vector cSequential = a.sumSequential(b);

                if (!c.equals(cSequential)) {
                    System.out.println("Sum error!");
                    System.out.println("Expected: " + cSequential);
                    System.out.println("Got:      " + c);
                } else {
                    System.out.println("Sum OK:   " + c);
                }

                int d = a.dot(b);
                int dotSequential = a.dotSequential(b);
                if (d != dotSequential) {
                    System.out.println("Dot error! Expected " + dotSequential + ", got " + d + ".");
                } else {
                    System.out.println("Dot OK: " + d);
                }
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            System.err.println("computations interrupted");
        }
    }
}
