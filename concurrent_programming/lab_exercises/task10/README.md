### Task Description

The pre-Christmas period is always busy, especially when a deadline is approaching and you are an elf in the toy factory.

Help the elves working in Santa's factory prepare gifts for the holidays.

Gifts in the factory are produced by `N_ELFS` elves and delivered to Santa by `N_REINDEERS` reindeers. Each elf produces `LOOPS` gifts; the function `int produce()` creates a single gift. After producing a gift, each elf puts it into the warehouse and starts producing the next gift. The warehouse is a **circular buffer** of size `BUFF_SIZE`. Gifts from the warehouse are picked up by reindeers and delivered to Santa via the function `void deliver(int gift)`.

Complete (in the TODO sections) the code in the file `gifts_skeleton.c` so that elves and reindeers can work safely.  
Use **shared memory** and **semaphores** to manage the warehouse. Ensure proper concurrency and remember to release system resources after the task is finished.
