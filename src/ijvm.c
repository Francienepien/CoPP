#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions


// see ijvm.h for descriptions of the below functions

ijvm* init_ijvm(char *binary_path, FILE* input , FILE* output) 
{
  // do not change these first three lines
  ijvm* m = (ijvm *) malloc(sizeof(ijvm));
  // note that malloc gives you memory, but gives no guarantees on the initial
  // values of that memory. It might be all zeroes, or be random data.
  // It is hence important that you initialize all variables in the ijvm
  // struct and do not assume these are set to zero.
  m->in = input;
  m->out = output;
  
  m->programCounter=0;
  m->finished = false;
  FILE * binaryFile = fopen(binary_path, "rb");

  byte_t * magicNumberBin = (byte_t *) malloc(sizeof(byte_t) * 4);
  fread(magicNumberBin, sizeof(byte_t), 4, binaryFile);

  int fileMagicNumber = read_uint32(magicNumberBin);
  if (fileMagicNumber != MAGIC_NUMBER) {
    return NULL;
  }

  fseek(binaryFile, 4, SEEK_CUR);
  m->constantSize = (byte_t *) malloc(sizeof(byte_t) * 4);
  fread(m->constantSize, sizeof(byte_t), 4, binaryFile);

  int constantSizeInt = read_uint32(m->constantSize);
  m->constantData = (word_t *) malloc(sizeof(byte_t) * constantSizeInt);
  fread(m->constantData, sizeof(byte_t), constantSizeInt, binaryFile);

  fseek(binaryFile, 4, SEEK_CUR);
  m->textSize = (byte_t *) malloc(sizeof(byte_t) * 4);
  fread(m->textSize, sizeof(byte_t), 4, binaryFile);
  
  int textSizeInt = get_text_size(m);
  m->textData = (byte_t *) malloc(sizeof(byte_t) * textSizeInt);
  fread(m->textData, sizeof(byte_t), textSizeInt, binaryFile);

  fclose(binaryFile);

  m->stack = createStack(4);
  m->instructionCount = textSizeInt;

  return m;
}

void destroy_ijvm(ijvm* m) 
{
  free(m->constantSize);
  free(m->constantData);
  free(m->textSize);
  free(m->textData);

  free(m);
}

byte_t *get_text(ijvm* m) 
{
  return m->textData;
}

unsigned int get_text_size(ijvm* m) 
{
  int size = read_uint32(m->textSize);
  return size;
}

word_t get_constant(ijvm* m,int i) 
{
  return swap_uint32(m->constantData[i]);
}

unsigned int get_program_counter(ijvm* m) 
{
  // TODO: implement me
  return m->programCounter;
}

word_t tos(ijvm* m) 
{
  // this operation should NOT pop (remove top element from stack)
  // TODO: implement me
  return top(m->stack);
}

bool finished(ijvm* m) 
{
  // TODO: implement me
  return m->finished;
}

word_t get_local_variable(ijvm* m, int i) 
{
  // TODO: implement me
  return 0;
}

void step(ijvm* m) 
{
  // TODO: implement me
  word_t operand1, operand2, result, var;
  if (m->instructionCount-1 == m->programCounter) {
    m->finished = true;
    return;
  }

  switch(get_instruction(m)) {
    case 0x10: //BIPUSH
      m->programCounter++;
      int8_t signed_value = (int8_t)get_instruction(m);
      word_t value = (word_t) signed_value;
      push(m->stack, value);
      m->programCounter++;
      break;
    case 0x59: //DUP
      var = top(m->stack);
      push(m->stack, var);
      m->programCounter++;
      break;
    case 0x60: //IADD
      operand1 = pop(m->stack);
      operand2 = pop(m->stack);
      result = operand1 + operand2;
      push(m->stack, result);
      m->programCounter++;
      break;
    case 0x7E: //IAND
      operand1 = pop(m->stack);
      operand2 = pop(m->stack);
      result = operand1 & operand2;
      push(m->stack, result);
      m->programCounter++;
      break;
    case 0xB0: //IOR
      operand1 = pop(m->stack);
      operand2 = pop(m->stack);
      result = operand1 | operand2;
      push(m->stack, result);
      m->programCounter++;
      break;
    case 0x64: //ISUB
      operand1 = pop(m->stack);
      operand2 = pop(m->stack);
      result = operand2 - operand1;
      push(m->stack, result);
      m->programCounter++;
      break;
    case 0x00: //NOP
      m->programCounter++;
      break;
    case 0x57: //POP
      pop(m->stack);
      m->programCounter++;
      break;
    case 0x5F: //SWAP
      operand1 = pop(m->stack);
      operand2 = pop(m->stack);
      push(m->stack, operand1);
      push(m->stack, operand2);
      m->programCounter++;
      break;
    case 0xFE: //ERR
      fprintf(m->out, "%s", "error");
      m->finished = true;
      m->programCounter++;
      break;
    case 0xFF: //HALT
      m->finished = true;
      break;
    case 0xFC: //IN
      var = (word_t) fgetc(m->in);
      push(m->stack, var);
      m->programCounter++;
      break;
    case 0xFD: //OUT
      var = pop(m->stack);
      fprintf(m->out, "%c", var);
      m->programCounter++;
      break;
    case 0xA7: //GOTO
      break;
    case 0x99: //IFEQ
      break;
    case 0x9B: //IFLT
      break;
    case 0x9F: //IF_ICMPEQ
      break;
    default:
    break;
  }

}

byte_t get_instruction(ijvm* m) 
{ 
  return get_text(m)[get_program_counter(m)]; 
}

ijvm* init_ijvm_std(char *binary_path) 
{
  return init_ijvm(binary_path, stdin, stdout);
}

void run(ijvm* m) 
{
  while (!finished(m)) 
  {
    step(m);
  }
}


// Below: methods needed by bonus assignments, see ijvm.h
// You can leave these unimplemented if you are not doing these bonus 
// assignments.

int get_call_stack_size(ijvm* m) 
{
   // TODO: implement me if doing tail call bonus
   return 0;
}


// Checks if reference is a freed heap array. Note that this assumes that 
// 
bool is_heap_freed(ijvm* m, word_t reference) 
{
   // TODO: implement me if doing garbage collection bonus
   return 0;
}
