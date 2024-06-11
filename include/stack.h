
#ifndef STACK_H
#define STACK_H

#include "ijvm_types.h"

typedef struct STACK {
    int capacity;
    int load;
    word_t top;
    word_t * stackPointer;
    word_t * basePointer;
} Stack;

Stack* createStack(int capacity);

void push(Stack* programStack, word_t value);

word_t pop(Stack* programStack);

word_t top(Stack* programStack);

void resizeStack (Stack* stack);

#endif
