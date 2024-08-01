#include "process.h"
#include "commonprocess.h"

#define MAX_TASKS 10
#define STACK_SIZE 1024
#define TIME_SLICE 100
#define MAX_PRIORITY 10

static task_t *current_task = NULL;
static task_t *task_list = NULL;
static task_t *task_end = NULL;

void task_init(void) {
    current_task = NULL;
    task_list = NULL;
    task_end = NULL;
}

task_t *task_create(void (*entry)(void), uint32_t *stack, uint32_t stack_size, uint8_t priority) {
    task_t *task = (task_t *)malloc(sizeof(task_t));
    if (!task) return NULL;

    // Set up stack
    uint32_t *stack_top = stack + stack_size - 1;
    *(stack_top--) = (uint32_t)entry; // Entry point
    *(stack_top--) = 0x01000000;     // xPSR (Thumb mode bit set)
    *(stack_top--) = 0xFFFFFFFD;     // Return address (exception return address)

    task->stack_pointer = stack_top;
    task->state = TASK_READY;
    task->priority = priority;
    task->ticks_remaining = TIME_SLICE; // Initialize with time slice
    task->next = NULL;

    // Insert task into list by priority
    if (!task_list || priority > task_list->priority) {
        task->next = task_list;
        task_list = task;
    } else {
        task_t *prev = task_list;
        while (prev->next && prev->next->priority >= priority) {
            prev = prev->next;
        }
        task->next = prev->next;
        prev->next = task;
    }
    if (!task_end || priority >= task_end->priority) {
        task_end = task;
    }

    return task;
}

static void context_switch(void) {
    task_t *prev_task = current_task;
    if (!current_task) {
        current_task = task_list;
    } else {
        asm volatile (
            "mrs r0, psp\n"
            "str r0, [%0]\n"
            : "=r" (prev_task->stack_pointer)
        );

        current_task = current_task->next ? current_task->next : task_list;
        while (current_task->state != TASK_READY) {
            current_task = current_task->next ? current_task->next : task_list;
        }
    }
  
    asm volatile (
        "ldr r0, [%0]\n"
        "msr psp, r0\n"
        : : "r" (current_task->stack_pointer)
    );
}

void timer_handler(void) {
    if (current_task) {
        current_task->ticks_remaining--;
        if (current_task->ticks_remaining == 0) {
            current_task->ticks_remaining = TIME_SLICE;
            task_yield();
        }
    } else {
        task_yield();
    }
}

void task_yield(void) {
    context_switch();
}
