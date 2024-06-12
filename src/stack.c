#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions
#include <assert.h>

#include "stack.h"

Stack* createStack(int capacity) {
    Stack* stack = (Stack *) malloc(sizeof(Stack));
    stack->basePointer = malloc(sizeof(word_t)*capacity);
    stack->capacity=capacity;
    stack->load = 0;
    return stack;
}

void push(Stack* stack, word_t value) {
    if (stack->load < stack->capacity) {
        stack->basePointer[stack->load] = value;
        stack->load++;
    }
    else {
        resizeStack(stack);
        push(stack, value);
    }
}

word_t pop(Stack* stack) {
    assert(stack->load > 0);
    word_t var = top(stack);
    stack->load--;
    return var;
}

word_t top(Stack* stack) {
    assert(stack->load > 0);
    return stack->basePointer[stack->load-1];
}

void resizeStack (Stack* stack) {
    word_t * tmp = realloc(stack->basePointer, (sizeof(word_t)*(stack->capacity+4)));
    if (tmp != NULL) {
        stack->basePointer = tmp;
    }
    stack->capacity += 4;
}
