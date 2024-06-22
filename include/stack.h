
#ifndef STACK_H
#define STACK_H

#include "ijvm_types.h"

typedef struct STACK {
    int capacity;
    int load;
    word_t top;
    word_t stackPointer;
    word_t * basePointer, * reallocPointer;
    word_t baseIndex;
} Stack;

Stack* createStack(int capacity);

void push(Stack* stack, word_t value);

void load_index(Stack* stack, int index);

word_t pop(Stack* stack);

void store_index(Stack* stack, int index, word_t value);

word_t top(Stack* stack);

void resizeStack (Stack* stack);

void adjust_stack_pointer(Stack* stack, int index);

#endif
