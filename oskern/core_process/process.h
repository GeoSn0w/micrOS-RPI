#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED
} task_state_t;

typedef struct task {
    uint32_t *stack_pointer;
    task_state_t state;
    struct task *next;
    uint8_t priority;
    uint32_t ticks_remaining;
} task_t;

void task_init(void);
task_t *task_create(void (*entry)(void), uint32_t *stack, uint32_t stack_size, uint8_t priority);
void task_yield(void);
void timer_handler(void);

#endif // PROCESS_H
