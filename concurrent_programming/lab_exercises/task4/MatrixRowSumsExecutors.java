package lab05.assignments;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.function.IntBinaryOperator;


public class MatrixRowSumsPooled {
    private static final int N_ROWS = 10;
    private static final int N_COLUMNS = 100;
    private static final int N_THREADS = 4;


    private static IntBinaryOperator matrixDefinition = (row, col) -> {
        int a = 2 * col + 1;
        return (row + 1) * (a % 4 - 2) * a;
    };

    private static void printRowSumsSequentially() {
        for (int r = 0; r < N_ROWS; ++r) {
            int sum = 0;
            for (int c = 0; c < N_COLUMNS; ++c) {
                sum += matrixDefinition.applyAsInt(r, c);
            }
            System.out.println(r + " -> " + sum);
        }
    }

    public static void printRowSumsInParallel() throws InterruptedException {
        ExecutorService pool = Executors.newFixedThreadPool(N_THREADS);

        try {
            // Lista list futures, jedna lista dla każdego wiersza
            List<List<Future<Integer>>> rowFutures = new ArrayList<>();

            // Tworzymy pustą listę dla każdego wiersza
            for (int r = 0; r < N_ROWS; r++) {
                rowFutures.add(new ArrayList<>());
            }

            // Przygotowanie zadań dla puli wątków
            for (int r = 0; r < N_ROWS; ++r) {
                for (int c = 0; c < N_COLUMNS; ++c) {
                    // Przypisujemy zadanie do odpowiedniego wiersza
                    rowFutures.get(r).add(pool.submit(new Task(r, c)));
                }
            }

            // Sumowanie wyników w głównym wątku
            int[] rowSums = new int[N_ROWS];

            // Pobieramy wyniki dla każdego wiersza
            for (int r = 0; r < N_ROWS; ++r) {
                List<Future<Integer>> futuresForRow = rowFutures.get(r);
                for (Future<Integer> future : futuresForRow) {
                    try {
                        int elementValue = future.get(); // Oczekiwanie na wynik obliczenia elementu
                        rowSums[r] += elementValue; // Dodajemy do sumy wiersza
                    } catch (InterruptedException e) {
                        // Obsługuje przerwanie wątku
                        System.err.println("Thread was interrupted while waiting for the result.");
                        Thread.currentThread().interrupt(); // Przywrócenie statusu przerwania
                        pool.shutdownNow();  // Przerwanie wszystkich wątków roboczych
                        return;  // Kończymy działanie metody
                    } catch (ExecutionException e) {
                        // Obsługuje wyjątek związany z błędem w zadaniu
                        System.err.println("An error occurred while executing the task: " + e.getCause());
                    }
                }
            }
            
            // Wypisujemy sumy wierszy
            for (int r = 0; r < N_ROWS; ++r) {
                System.out.println(r + " -> " + rowSums[r]);
            }
        } finally {
            pool.shutdown();
        }
    }

    private static class Task implements Callable<Integer> {
        private final int rowNo;
        private final int columnNo;

        private Task(int rowNo, int columnNo) {
            this.rowNo = rowNo;
            this.columnNo = columnNo;
        }

        @Override
        public Integer call() {
            // Sprawdzenie, czy wątek został przerwany przed rozpoczęciem
            if (Thread.currentThread().isInterrupted()) {
                System.err.println("Thread interrupted before task start: row " + rowNo + ", column " + columnNo);
                return 0;
            }

            // Obliczanie wartości elementu macierzy
            int result = matrixDefinition.applyAsInt(rowNo, columnNo);

            // Sprawdzenie przerwania po
            if (Thread.currentThread().isInterrupted()) {
                System.err.println("Thread interrupted after task: row " + rowNo + ", column " + columnNo);
                return 0;
            }

            return result;
        }
    }

    public static void main(String[] args) {
        try {
            System.out.println("-- Sequentially --");
            printRowSumsSequentially();
            System.out.println("-- In a FixedThreadPool --");
            printRowSumsInParallel();
            System.out.println("-- End --");
        } catch (InterruptedException e) {
            System.err.println("Main interrupted.");
        }
    }
}
