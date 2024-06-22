#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions
#include <assert.h>

#include "stack.h"

Stack* createStack(int capacity) {
    Stack* stack = (Stack *) malloc(sizeof(Stack));
    stack->basePointer = malloc(sizeof(word_t)*capacity);
    stack->stackPointer = stack->basePointer;
    stack->capacity = capacity;
    stack->load = 0;
    return stack;
}

void push(Stack* stack, word_t value) {
    if (stack->load < stack->capacity) {
        *stack->stackPointer = value;
        stack->stackPointer++;
        stack->load++;
    }
    else {
        resizeStack(stack);
        push(stack, value);
    }
}

void load_index (Stack* stack, int index) {
    if (stack->load < stack->capacity) {
        *stack->stackPointer = stack->basePointer[index];
        stack->stackPointer++;
        stack->load++;
    }
    else {
        resizeStack(stack);
        load_index(stack, index);
    }
}

word_t pop(Stack* stack) {
    assert(stack->load > 0);
    word_t var = top(stack);
    stack->stackPointer--;
    stack->load--;
    return var;
}

void store_index(Stack* stack, int index, word_t value) {
    stack->basePointer[index] = value;
}

word_t top(Stack* stack) {
    assert(stack->load > 0);
    return *(stack->stackPointer - 1);
}

void resizeStack (Stack* stack) {
    word_t * tmp = realloc(stack->basePointer, (sizeof(word_t)*(stack->capacity+4)));
    assert (tmp != NULL);
    stack->basePointer = tmp;
    stack->stackPointer = stack->basePointer + stack->load;
    
    stack->capacity += 4;
}
