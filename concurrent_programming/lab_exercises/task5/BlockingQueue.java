package lab06.assignments;

import java.util.LinkedList;
import java.util.Queue;

public class BlockingQueue<T> {
    private final int capacity;
    private final Queue<T> queue;

    public BlockingQueue(int capacity) {
        this.capacity = capacity;
        this.queue = new LinkedList<>();
    }

    public synchronized T take() throws InterruptedException {
        // Jeśli kolejka jest pusta, wątek oczekuje aż element zostanie dodany
        while (queue.isEmpty()) {
            wait();
        }
        // Pobranie elementu z niepustej kolejki
        T item = queue.remove();
        // Obudź "śpiące" na "wait" wątki, bo jest miejsce na nowy element
        notifyAll();
        return item;
    }

    public synchronized void put(T item) throws InterruptedException {
        // Jeśli kolejka jest pełna, wątek oczekuje aż miejsce się zwolni
        while (queue.size() == capacity) {
            wait();
        }
        // Dodanie elementu do niepełnej kolejki
        queue.add(item);
        // Obudź "śpiące" na "wait" wątki, bo dostępny jest nowy element
        notifyAll();
    }

    public synchronized int getSize() {
        // Zwraca aktualny rozmiar kolejki
        return queue.size();
    }

    public int getCapacity() {
        return capacity;
    }
}