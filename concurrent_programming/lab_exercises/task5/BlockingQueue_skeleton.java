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
        throw new RuntimeException("Not implemented.");
    }

    public synchronized void put(T item) throws InterruptedException {
        throw new RuntimeException("Not implemented.");
    }

    public synchronized int getSize() {
        throw new RuntimeException("Not implemented.");
    }

    public int getCapacity() {
        return capacity;
    }
}