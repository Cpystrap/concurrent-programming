#include <stddef.h>
#include <stdlib.h>
#include "common/io.h"
#include "common/sumset.h"

typedef struct {
    Sumset a;
    Sumset* b;
    // if I am the last child of my father, pop the father
    bool last;
} StackFrame;

typedef struct {
    StackFrame* frames;
    size_t size;
    size_t capacity;
} Stack;

void stack_init(Stack* stack, size_t d) {
    // It can be shown that this size of stack is enough 
    size_t capacity = d * d;
    stack->frames = malloc(sizeof(StackFrame) * capacity);
    if (!stack->frames) {
        exit(1);
    }
    stack->size = 0;
    stack->capacity = capacity;
}

void stack_free(Stack* stack) {
    free(stack->frames);
}

void stack_push(Stack* stack, const Sumset* a, Sumset* b, bool last) {
    if (stack->size == stack->capacity) {
        stack_free(stack);
        exit(1);
    }
    stack->frames[stack->size++] = (StackFrame){ *a, b, last };
}

void stack_push_create(Stack* stack, const Sumset* a, Sumset* b, bool last, size_t i) {
    if (stack->size == stack->capacity) {
        stack_free(stack);
        exit(1);
    }
    sumset_add(&stack->frames[stack->size].a, a, i);
    stack->frames[stack->size].b = b;
    stack->frames[stack->size].last = last;
    stack->size++;
}

void stack_pop(Stack* stack) {
    if (stack->size > 0) {
        stack->size--;
    }
}

static Solution best_solution;

void solve_nonrecursive(InputData* input_data) {
    Stack stack;
    stack_init(&stack, input_data->d);

    stack_push(&stack, &input_data->a_start, &input_data->b_start, false);

    while (stack.size > 0) {
        bool leaf = true;   // checks if a given sumsets will produce new
        // case or not (then it is a leaf)
        bool last = true;   // info which of our children will be the last to be
        // calculated and should pop ancestors if they were also last children
        // of theirs
        Sumset* a = &stack.frames[stack.size - 1].a;
        Sumset* b = stack.frames[stack.size - 1].b;

        if (stack.frames[stack.size - 1].a.sum > stack.frames[stack.size - 1].b->sum) {
            Sumset* temp = a;
            a = b;
            b = temp;
        }

        if (is_sumset_intersection_trivial(a, b)) {   // s(a) ∩ s(b) = {0}.
            // we change the order in for to have the same
            // dfs as in reference
            for (size_t i = input_data->d; i >= a->last; --i) {
                if (!does_sumset_contain(b, i)) {
                    leaf = false;

                    stack_push_create(&stack, a, b, last, i);

                    if (last)
                        last = false;
                }
            }
        } else if ((a->sum == b->sum) && (get_sumset_intersection_size(a, b) == 2)) { // s(a) ∩ s(b) = {0, ∑b}.
            if (b->sum > best_solution.sum)
                solution_build(&best_solution, input_data, a, b);
        }

        // leafs should pop themselves
        // if they are last leafs, they should pop their father
        // if they are last leafs and their father was also last,
        // grandfather also should be popped and if grandfather
        // was also last etc.
        if (leaf) {
            last = stack.frames[stack.size - 1].last;
            stack_pop(&stack);
            while (last) {
                last = stack.frames[stack.size - 1].last;
                stack_pop(&stack);
            }
        }
    }

    stack_free(&stack);
}

int main() {
    InputData input_data;
    input_data_read(&input_data);
    // input_data_init(&input_data, 1, 3, (int[]){0}, (int[]){0});

    solution_init(&best_solution);

    solve_nonrecursive(&input_data);

    solution_print(&best_solution);
    return 0;
}