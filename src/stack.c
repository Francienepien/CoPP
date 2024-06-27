#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions
#include <assert.h>

#include "stack.h"

Stack *create_stack(int capacity)
{
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->base_pointer = malloc(sizeof(word_t) * capacity);
    stack->base_index = 0;
    stack->stack_pointer = 0;
    stack->capacity = capacity;
    return stack;
}

void push(Stack *stack, word_t value)
{
    if (stack->stack_pointer < stack->capacity)
    {
        stack->base_pointer[stack->stack_pointer] = value;
        stack->stack_pointer++;
    }
    else
    {
        resize_stack(stack);
        push(stack, value);
    }
}

word_t pop(Stack *stack)
{
    assert(stack->stack_pointer >= 0);
    word_t var = top(stack);
    stack->stack_pointer--;
    return var;
}

word_t top(Stack *stack)
{
    assert(stack->stack_pointer >= 0);
    return stack->base_pointer[stack->stack_pointer - 1];
}

void resize_stack(Stack *stack)
{
    word_t *tmp = realloc(stack->base_pointer, (sizeof(word_t) * (stack->capacity + 4)));
    assert(tmp != NULL);
    stack->base_pointer = tmp;
    stack->capacity += 4;
}
