
#ifndef STACK_H
#define STACK_H

#include "ijvm_types.h"

typedef struct STACK
{
    unsigned int capacity, load, stack_pointer, base_index;
    word_t *base_pointer;
} Stack;

Stack *create_stack(int capacity);

void push(Stack *stack, word_t value);

word_t pop(Stack *stack);

word_t top(Stack *stack);

void resize_stack(Stack *stack);

void adjust_stack_pointer(Stack *stack, int index);

#endif
