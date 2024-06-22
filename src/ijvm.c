#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions
#include <assert.h>


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

  m->stack = createStack(260);
  for (int i = 0; i < 256; i++) {
    push(m->stack, 0);
  }
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
  return m->stack->basePointer[m->stack->baseIndex+i];
}

void step(ijvm* m) 
{
  // TODO: implement me
  word_t operand1, operand2, result, var;
  int16_t argument, location;
  int8_t signedValue;
  if (m->instructionCount-1 == m->programCounter) {
    m->finished = true;
    return;
  }

  switch(get_instruction(m)) {
    case 0x10: //BIPUSH
      m->programCounter++;
      signedValue = (int8_t)get_instruction(m);
      word_t value = (word_t) signedValue;
      dprintf("value: %d\n\n", value);
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
      assert(var != EOF);
      push(m->stack, var);
      m->programCounter++;
      break;
    case 0xFD: //OUT
      var = pop(m->stack);
      fprintf(m->out, "%c", var);
      m->programCounter++;
      break;
    case 0xA7:; //GOTO
      argument = get_short(m);
      m->programCounter += argument;
      break;
    case 0x99: //IFEQ
      if (pop(m->stack) == 0) {
        argument = get_short(m);
        m->programCounter += argument;
      }
      else {
        m->programCounter += 3;
      }
      break;
    case 0x9B: //IFLT
      if (pop(m->stack) < 0) {
        argument = get_short(m);
        m->programCounter += argument;
      }
      else {
        m->programCounter += 3;
      }
      break;
    case 0x9F: //IF_ICMPEQ
      operand1 = pop(m->stack);
      operand2 = pop(m->stack);
      if (operand1 == operand2) {
        argument = get_short(m);
        m->programCounter += argument;
      }
      else {
        m->programCounter += 3;
      }
      break;
    case 0x13: //LDC_W
      argument = get_short(m);
      push(m->stack, get_constant(m, argument));
      m->programCounter += 3;
      break;
    case 0x15: //ILOAD
      m->programCounter++;
      location = (int16_t) get_instruction(m);
      load_index(m->stack, location);
      m->programCounter++;
      break;
    case 0x36: //ISTORE
      m->programCounter++;
      location = (int16_t) get_instruction(m);
      store_index(m->stack, location, pop(m->stack));
      m->programCounter++;
      break;
    case 0x84: //IINC
      m->programCounter++;
      location = (int16_t) get_instruction(m);
      m->programCounter++;
      signedValue = get_instruction(m);
      m->stack->basePointer[location] += signedValue;
      m->programCounter++;
      break;
    case 0xC4: //WIDE
      m->programCounter++;
      switch(get_instruction(m)) {
        case 0x15: //ILOAD
          dprintf("YIPPEE\n");
          location = get_short(m);
          m->programCounter += 2;
          load_index(m->stack, location);
          m->programCounter++;
          break;
        case 0x36: //ISTORE
          location = get_short(m);
          m->programCounter += 2;
          store_index(m->stack, location, pop(m->stack));
          m->programCounter++;
          break;
        case 0x84: //IINC
          location = get_short(m);
          m->programCounter += 3;
          signedValue = get_instruction(m);
          m->stack->basePointer[location] += signedValue;
          m->programCounter++;
          break;
      }
      break;
    
    case 0xB6:; //INVOKEVIRTUAL
      int oldProgramCounter = m->programCounter + 3;
      location = get_short(m); //fetch objref pos
      var = get_constant(m, location); //fetch programcounter pointed to by objref

      m->programCounter = var - 1;

      operand1 = get_short(m); //no. of arguments (including objref)
      m->programCounter  += 2; 
      operand2 = get_short(m); //no. of local variables
      m->programCounter  += 3;

      m->stack->stackPointer += operand2 + 1;

      push(m->stack, oldProgramCounter);
      push(m->stack, m->stack->baseIndex);

      m->stack->stackPointer -= 2 + operand1 + operand2;
      m->stack->baseIndex = m->stack->stackPointer;
      dprintf("stackpointer: %d\nbaseindex: %d\n\n", m->stack->stackPointer, m->stack->baseIndex);

      push(m->stack, operand1 + operand2);

      m->stack->stackPointer += operand1 + operand2 + 1;
      break;
    
    case 0xAC: //IRETURN
      m->returnValue = pop(m->stack);

      word_t LV = get_local_variable(m, 0);

      dprintf("stackpointer: %d\n", m->stack->baseIndex + LV);

      m->stack->stackPointer = m->stack->baseIndex + LV + 1;

      m->programCounter = pop(m->stack);

      dprintf("DIKKE PIK: %d\n",m->programCounter);

      m->stack->stackPointer += 2;
      word_t baseIndex = pop(m->stack);
      m->stack->stackPointer = m->stack->baseIndex;
      m->stack->baseIndex = baseIndex;
      m->stack->basePointer = &(m->stack->basePointer[baseIndex]);

      push(m->stack, m->returnValue);

      break;
    
    default:
    dprintf("OPCODE doesn't exist\n");
    break;
  }

}

int16_t get_short(ijvm* m) {
  m->programCounter++;
  int8_t tmp = get_instruction(m);
  m->programCounter++;
  int16_t result = (int16_t) tmp << 8 | (int16_t) get_instruction(m);
  m->programCounter -= 2;
  return result;
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
