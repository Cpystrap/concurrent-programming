#include "future_combinators.h"

#include <err.h>
#include <stdlib.h>

#include "future.h"
#include "waker.h"
#include "executor.h"

/* ===================== ThenFuture ===================== */

/**
 * Progress function for ThenFuture.
 *
 * First, progress the first future (fut1). If it completes successfully,
 * assign its result to fut2->arg and mark fut1 as completed.
 * Then, progress fut2 – if it fails, return FAILURE with the appropriate code,
 * and if it succeeds, return COMPLETED.
 */
static FutureState then_future_progress(Future* fut, Mio* mio, Waker waker) {
    ThenFuture* self = (ThenFuture*)fut;

    // If the first future has not completed yet, progress it.
    if (!self->fut1_completed) {
        FutureState state1 = self->fut1->progress(self->fut1, mio, waker);
        if (state1 == FUTURE_COMPLETED) {
            self->fut1_completed = true;
            // Pass the result of the first future as an argument to the second.
            self->fut2->arg = self->fut1->ok;
        } else if (state1 == FUTURE_FAILURE) {
            fut->errcode = THEN_FUTURE_ERR_FUT1_FAILED;
            return FUTURE_FAILURE;
        } else {
            return FUTURE_PENDING;
        }
    }

    // Progress the second future.
    FutureState state2 = self->fut2->progress(self->fut2, mio, waker);
    if (state2 == FUTURE_FAILURE) {
        fut->errcode = THEN_FUTURE_ERR_FUT2_FAILED;
    }
    else if (state2 == FUTURE_COMPLETED) {
        // If the second future completed successfully, set the result.
        fut->ok = self->fut2->ok;
    }

    return state2;
}

/**
 * Public function for creating a ThenFuture.
 */
ThenFuture future_then(Future* fut1, Future* fut2) {
    ThenFuture tf;
    tf.base = future_create(then_future_progress);
    tf.fut1 = fut1;
    tf.fut2 = fut2;
    tf.fut1_completed = false;
    return tf;
}

/* ===================== JoinFuture ===================== */

/* Definition of a wrapper for join – we wrap each child. */
typedef struct JoinWrapperFuture {
    Future base;         // Base future – must be the first field.
    bool is_fut1;        // true if wrapping fut1; false for fut2.
    JoinFuture* parent;  // Pointer to the parent (JoinFuture) that spawned us.
    Waker waker;         // Waker passed when calling the parent's progress.
} JoinWrapperFuture;

/*
 * Progress function for the JoinWrapperFuture wrapper.
 * For the wrapper, check whether it represents fut1 or fut2.
 * Then call progress on the corresponding child.
 * If the result is not pending, store it in the parent and, if the other wrapper
 * is also not pending, wake the waker.
 */
FutureState join_wrapper_progress(Future* fut, Mio* mio, Waker external_waker) {
    JoinWrapperFuture* wrapper = (JoinWrapperFuture*)fut;
    JoinFuture* parent = wrapper->parent;
    FutureState state;
    if (wrapper->is_fut1) {
        state = parent->fut1->progress(parent->fut1, mio, external_waker);
        if (state != FUTURE_PENDING) {
            parent->fut1_completed = state;
            parent->result.fut1.errcode = parent->fut1->errcode;
            parent->result.fut1.ok = parent->fut1->ok;
            if (parent->fut2_completed != FUTURE_PENDING) {
                waker_wake(&wrapper->waker);
            }
        }
    } else {
        state = parent->fut2->progress(parent->fut2, mio, external_waker);
        if (state != FUTURE_PENDING) {
            parent->fut2_completed = state;
            parent->result.fut2.errcode = parent->fut2->errcode;
            parent->result.fut2.ok = parent->fut2->ok;
            if (parent->fut1_completed != FUTURE_PENDING) {
                waker_wake(&wrapper->waker);
            }
        }
    }
    return state;
}

/*
 * Progress function for JoinFuture (the parent).
 * If the children have not been wrapped yet, create two wrappers for fut1 and fut2,
 * schedule their execution, and return FUTURE_PENDING.
 * When the wrappers complete, check their results and return the final state.
 */
static FutureState join_future_progress(Future* fut, Mio* mio, Waker waker) {
    JoinFuture* self = (JoinFuture*)fut;

    // If any child is still in the FUTURE_PENDING state, the children have not been wrapped yet.
    if (self->fut1_completed == FUTURE_PENDING) {
        // Create wrapper for fut1.
        JoinWrapperFuture* wrapper1 = malloc(sizeof(JoinWrapperFuture));
        if (!wrapper1)
            fatal("malloc join wrapper failed");
        *wrapper1 = (JoinWrapperFuture){
            .base = future_create(join_wrapper_progress),
            .is_fut1 = true,
            .parent = self,
            .waker = waker  // copy waker
        };

        // Create wrapper for fut2.
        JoinWrapperFuture* wrapper2 = malloc(sizeof(JoinWrapperFuture));
        if (!wrapper2) {
            free(wrapper1);
            fatal("malloc join wrapper failed");
        }
        *wrapper2 = (JoinWrapperFuture){
            .base = future_create(join_wrapper_progress),
            .is_fut1 = false,
            .parent = self,
            .waker = waker
        };

        // Schedule the execution of both wrappers.
        Executor* exec = waker.executor;
        executor_spawn(exec, (Future*)wrapper1);
        executor_spawn(exec, (Future*)wrapper2);
        return FUTURE_PENDING;
    }

    // Both children have completed – set the final result.
    if (self->fut1_completed == FUTURE_FAILURE || self->fut2_completed == FUTURE_FAILURE) {
        if (self->fut1_completed == FUTURE_FAILURE && self->fut2_completed == FUTURE_FAILURE)
            fut->errcode = JOIN_FUTURE_ERR_BOTH_FUTS_FAILED;
        else if (self->fut1_completed == FUTURE_FAILURE)
            fut->errcode = JOIN_FUTURE_ERR_FUT1_FAILED;
        else
            fut->errcode = JOIN_FUTURE_ERR_FUT2_FAILED;
        return FUTURE_FAILURE;
    }

    // Both completed successfully.
    fut->ok = self->result.fut1.ok; // the join result is not specified in the original description when both children succeed
    return FUTURE_COMPLETED;
}

/**
 * Public function for creating a JoinFuture.
 */
JoinFuture future_join(Future* fut1, Future* fut2) {
    JoinFuture jf;
    jf.base = future_create(join_future_progress);
    jf.fut1 = fut1;
    jf.fut2 = fut2;
    jf.fut1_completed = FUTURE_PENDING;
    jf.fut2_completed = FUTURE_PENDING;
    jf.result.fut1.errcode = FUTURE_SUCCESS;
    jf.result.fut1.ok = NULL;
    jf.result.fut2.errcode = FUTURE_SUCCESS;
    jf.result.fut2.ok = NULL;
    return jf;
}

/* ===================== SelectFuture ===================== */

/*
 * Definition of a wrapper for SelectFuture.
 * Each wrapper wraps one of the children (fut1 or fut2).
 */
typedef struct SelectWrapperFuture {
    Future base;
    bool is_fut1;          // true if wrapping fut1, false – fut2.
    SelectFuture* parent;  // Pointer to the parent (SelectFuture) that spawned the wrappers.
    Waker waker;           // Waker passed when calling the parent's progress.
} SelectWrapperFuture;

/*
 * Progress function for the SelectWrapperFuture wrapper.
 *
 * If a future has already completed, do not call progress again.
 *
 * For the wrapper, determine whether it represents fut1 or fut2.
 * Call progress on the corresponding child.
 *
 * If the child returns COMPLETED:
 *   - set the parent to SELECT_COMPLETED_FUT1 (similarly for fut2) 
 *     and call waker_wake.
 *
 * If the child returns FAILURE:
 *   - If the parent was in the SELECT_COMPLETED_NONE state, set it to
 *     SELECT_FAILED_FUT1 or SELECT_FAILED_FUT2 accordingly.
 *   - If the other child has already returned FAILURE, set
 *     SELECT_FAILED_BOTH and call waker_wake.
 *
 * If the result is pending, return FUTURE_PENDING.
 */
FutureState select_wrapper_progress(Future* fut, Mio* mio, Waker external_waker) {
    SelectWrapperFuture* wrapper = (SelectWrapperFuture*)fut;
    SelectFuture* parent = wrapper->parent;

    // If the parent has already determined the result, do not progress further.
    if (parent->which_completed == SELECT_COMPLETED_FUT1 ||
            parent->which_completed == SELECT_COMPLETED_FUT2) {
        return FUTURE_COMPLETED;
    }

    FutureState state;
    if (wrapper->is_fut1) {
        state = parent->fut1->progress(parent->fut1, mio, external_waker);
        if (state == FUTURE_COMPLETED) {
            // If the parent was still SELECT_COMPLETED_NONE or had SELECT_FAILED_FUT2,
            // set it to COMPLETED_FUT1.
            parent->which_completed = SELECT_COMPLETED_FUT1;
            waker_wake(&wrapper->waker);
        }
        if (state == FUTURE_FAILURE) {
            if (parent->which_completed == SELECT_COMPLETED_NONE) {
                parent->which_completed = SELECT_FAILED_FUT1;
            } else if (parent->which_completed == SELECT_FAILED_FUT2) {
                parent->which_completed = SELECT_FAILED_BOTH;
                waker_wake(&wrapper->waker);
            }
        }
    } else {
        // wrapper for fut2
        state = parent->fut2->progress(parent->fut2, mio, external_waker);
        if (state == FUTURE_COMPLETED) {
            parent->which_completed = SELECT_COMPLETED_FUT2;
            waker_wake(&wrapper->waker);
        }
        if (state == FUTURE_FAILURE) {
            if (parent->which_completed == SELECT_COMPLETED_NONE) {
                parent->which_completed = SELECT_FAILED_FUT2;
            } else if (parent->which_completed == SELECT_FAILED_FUT1) {
                parent->which_completed = SELECT_FAILED_BOTH;
                waker_wake(&wrapper->waker);
            }
        }
    }

    return state;
}

/*
 * Progress function for SelectFuture (the parent).
 *
 * On the first call, if the parent is in the SELECT_COMPLETED_NONE state,
 * create wrappers for both children and schedule their execution (spawn).
 * Then return FUTURE_PENDING.
 * On subsequent calls, when the parent's state is no longer SELECT_COMPLETED_NONE,
 * return the result:
 *   - If which_completed is SELECT_COMPLETED_FUT1 or SELECT_COMPLETED_FUT2,
 *     set the result (ok) and return COMPLETED.
 *   - If which_completed is SELECT_FAILED_BOTH, set errcode and return FAILURE.
 */
static FutureState select_future_progress(Future* fut, Mio* mio, Waker waker) {
    SelectFuture* self = (SelectFuture*)fut;

    if (self->which_completed == SELECT_COMPLETED_NONE) {
        Executor* exec = waker.executor;
        // Create wrapper for fut1.
        SelectWrapperFuture* wrapper1 = malloc(sizeof(SelectWrapperFuture));
        if (!wrapper1)
            fatal("malloc select wrapper failed");
        *wrapper1 = (SelectWrapperFuture){
            .base = future_create(select_wrapper_progress),
            .is_fut1 = true,
            .parent = self,
            .waker = waker
        };

        // Create wrapper for fut2.
        SelectWrapperFuture* wrapper2 = malloc(sizeof(SelectWrapperFuture));
        if (!wrapper2) {
            free(wrapper1);
            fatal("malloc select wrapper failed");
        }
        *wrapper2 = (SelectWrapperFuture){
            .base = future_create(select_wrapper_progress),
            .is_fut1 = false,
            .parent = self,
            .waker = waker
        };

        executor_spawn(exec, (Future*)wrapper1);
        executor_spawn(exec, (Future*)wrapper2);
        return FUTURE_PENDING;
    }

    // The children have already been "spawned". Now, on the next call,
    // return the final result based on the set state.
    if (self->which_completed == SELECT_FAILED_BOTH) {
        fut->errcode = self->fut1->errcode; // it is unspecified which errcode (1 or 2)
        return FUTURE_FAILURE;
    }

    // When complete.
    fut->ok = (self->which_completed == SELECT_COMPLETED_FUT1) ? self->fut1->ok : self->fut2->ok;
    return FUTURE_COMPLETED;
}

SelectFuture future_select(Future* fut1, Future* fut2) {
    SelectFuture sf;
    sf.base = future_create(select_future_progress);
    sf.fut1 = fut1;
    sf.fut2 = fut2;
    sf.which_completed = SELECT_COMPLETED_NONE;
    return sf;
}