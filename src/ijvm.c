#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>

// see ijvm.h for descriptions of the below functions

ijvm *init_ijvm(char *binary_path, FILE *input, FILE *output)
{
  // do not change these first three lines
  ijvm *m = (ijvm *)malloc(sizeof(ijvm));
  // note that malloc gives you memory, but gives no guarantees on the initial
  // values of that memory. It might be all zeroes, or be random data.
  // It is hence important that you initialize all variables in the ijvm
  // struct and do not assume these are set to zero.
  m->in = input;
  m->out = output;

  m->program_counter = 0;
  FILE *binary_file = fopen(binary_path, "rb");

  byte_t *magic_number_bin = (byte_t *)malloc(sizeof(byte_t) * 4);
  fread(magic_number_bin, sizeof(byte_t), 4, binary_file);

  uint32_t file_magic_number = read_uint32(magic_number_bin);
  if (file_magic_number != MAGIC_NUMBER)
  {
    free(magic_number_bin);
    return NULL;
  }
  free(magic_number_bin);

  fseek(binary_file, 4, SEEK_CUR);
  m->constant_size = (byte_t *)malloc(sizeof(byte_t) * 4);
  fread(m->constant_size, sizeof(byte_t), 4, binary_file);

  uint32_t constant_size_int = read_uint32(m->constant_size);
  m->constant_data = (word_t *)malloc(sizeof(byte_t) * constant_size_int);
  fread(m->constant_data, sizeof(byte_t), constant_size_int, binary_file);

  fseek(binary_file, 4, SEEK_CUR);
  m->text_size = (byte_t *)malloc(sizeof(byte_t) * 4);
  fread(m->text_size, sizeof(byte_t), 4, binary_file);

  uint32_t text_size_int = get_text_size(m);
  m->text_data = (byte_t *)malloc(sizeof(byte_t) * text_size_int);
  fread(m->text_data, sizeof(byte_t), text_size_int, binary_file);

  fclose(binary_file);

  m->stack = create_stack(260);
  for (int i = 0; i < 256; i++)
  {
    push(m->stack, 0xCAFE);
  }
  m->instruction_count = text_size_int;

  return m;
}

void destroy_ijvm(ijvm *m)
{
  free(m->constant_size);
  free(m->constant_data);
  free(m->text_size);
  free(m->text_data);
  free(m->stack->base_pointer);
  free(m->stack);

  free(m);
}

byte_t *get_text(ijvm *m)
{
  return m->text_data;
}

unsigned int get_text_size(ijvm *m)
{
  int size = read_uint32(m->text_size);
  return size;
}

word_t get_constant(ijvm *m, int i)
{
  return swap_uint32(m->constant_data[i]);
}

unsigned int get_program_counter(ijvm *m)
{
  // TODO: implement me
  return m->program_counter;
}

word_t tos(ijvm *m)
{
  // this operation should NOT pop (remove top element from stack)
  // TODO: implement me
  return top(m->stack);
}

bool finished(ijvm *m)
{
  // TODO: implement me
  return m->program_counter >= m->instruction_count;
}

word_t get_local_variable(ijvm *m, int i)
{
  // TODO: implement me
  return m->stack->base_pointer[m->stack->base_index + i];
}

void step(ijvm *m)
{
  // TODO: implement me
  switch (get_instruction(m))
  {
  case OP_BIPUSH:
    call_bipush(m);
    break;
  case OP_DUP:
    call_dup(m);
    break;
  case OP_IADD:
    call_iadd(m);
    break;
  case OP_IAND:
    call_iand(m);
    break;
  case OP_IOR:
    call_ior(m);
    break;
  case OP_ISUB:
    call_isub(m);
    break;
  case OP_NOP:
    m->program_counter++;
    break;
  case OP_POP:
    pop(m->stack);
    m->program_counter++;
    break;
  case OP_SWAP:
    call_swap(m);
    break;
  case OP_ERR:
    call_err(m);
    break;
  case OP_HALT:
    m->program_counter = m->instruction_count;
    break;
  case OP_IN:
    call_in(m);
    break;
  case OP_OUT:
    call_out(m);
    break;
  case OP_GOTO:
    call_goto(m);
    break;
  case OP_IFEQ:
    call_ifeq(m);
    break;
  case OP_IFLT:
    call_iflt(m);
    break;
  case OP_IF_ICMPEQ:
    call_if_icmpeq(m);
    break;
  case OP_LDC_W:
    call_ldc_w(m);
    break;
  case OP_ILOAD:
    call_iload(m);
    break;
  case OP_ISTORE:
    call_istore(m);
    break;
  case OP_IINC:
    call_iinc(m);
    break;
  case OP_WIDE:
    call_wide(m);
    break;
  case OP_INVOKEVIRTUAL:
    call_invokevirtual(m);
    break;
  case OP_IRETURN:
    call_ireturn(m);
    break;
  case OP_TAILCALL:
    call_tailcall(m);
    break;
  default:
    assert(!"OPCODE doesn't exist\n");
    break;
  }

  return;
}

int16_t get_short(ijvm *m)
{
  m->program_counter++;
  int16_t tmp = get_instruction(m);
  m->program_counter++;
  int16_t result = (int16_t)tmp << 8 | (int16_t)get_instruction(m);
  m->program_counter -= 2;
  return result;
}

byte_t get_instruction(ijvm *m)
{
  return get_text(m)[get_program_counter(m)];
}

ijvm *init_ijvm_std(char *binary_path)
{
  return init_ijvm(binary_path, stdin, stdout);
}

void run(ijvm *m)
{
  while (!finished(m))
  {
    step(m);
  }
}

void call_bipush(ijvm *m)
{
  m->program_counter++;
  int8_t signed_value = (int8_t)get_instruction(m);
  word_t value = (word_t)signed_value;
  push(m->stack, value);
  m->program_counter++;
}

void call_dup(ijvm *m)
{
  word_t var = top(m->stack);
  push(m->stack, var);
  m->program_counter++;
}

void call_iadd(ijvm *m)
{
  word_t operand1 = pop(m->stack);
  word_t operand2 = pop(m->stack);
  word_t result = operand1 + operand2;
  push(m->stack, result);
  m->program_counter++;
}

void call_iand(ijvm *m)
{
  word_t operand1 = pop(m->stack);
  word_t operand2 = pop(m->stack);
  word_t result = operand1 & operand2;
  push(m->stack, result);
  m->program_counter++;
}

void call_ior(ijvm *m)
{
  word_t operand1 = pop(m->stack);
  word_t operand2 = pop(m->stack);
  word_t result = operand1 | operand2;
  push(m->stack, result);
  m->program_counter++;
}

void call_isub(ijvm *m)
{
  word_t operand1 = pop(m->stack);
  word_t operand2 = pop(m->stack);
  word_t result = operand2 - operand1;
  push(m->stack, result);
  m->program_counter++;
}

void call_swap(ijvm *m)
{
  word_t operand1 = pop(m->stack);
  word_t operand2 = pop(m->stack);
  push(m->stack, operand1);
  push(m->stack, operand2);
  m->program_counter++;
}

void call_err(ijvm *m)
{
  fprintf(m->out, "%s", "error");
  m->program_counter = m->instruction_count;
  m->program_counter++;
}

void call_in(ijvm *m)
{
  word_t var = (word_t)fgetc(m->in);
  if (var == EOF)
  {
    var = 0;
  }
  push(m->stack, var);
  m->program_counter++;
}

void call_out(ijvm *m)
{
  word_t var = pop(m->stack);
  fprintf(m->out, "%c", var);
  m->program_counter++;
}

void call_goto(ijvm *m)
{
  int16_t argument = get_short(m);
  m->program_counter += argument;
}

void call_ifeq(ijvm *m)
{
  if (pop(m->stack) == 0)
  {
    int16_t argument = get_short(m);
    m->program_counter += argument;
  }
  else
  {
    m->program_counter += 3;
  }
}

void call_iflt(ijvm *m)
{
  if (pop(m->stack) < 0)
  {
    int16_t argument = get_short(m);
    m->program_counter += argument;
  }
  else
  {
    m->program_counter += 3;
  }
}

void call_if_icmpeq(ijvm *m)
{
  word_t operand1 = pop(m->stack);
  word_t operand2 = pop(m->stack);
  if (operand1 == operand2)
  {
    int16_t argument = get_short(m);
    m->program_counter += argument;
  }
  else
  {
    m->program_counter += 3;
  }
}

void call_ldc_w(ijvm *m)
{
  uint16_t index = get_short(m);
  push(m->stack, get_constant(m, index));
  m->program_counter += 3;
}

void call_iload(ijvm *m)
{
  m->program_counter++;
  uint16_t location = get_instruction(m);
  push(m->stack, get_local_variable(m, location));
  m->program_counter++;
}

void call_istore(ijvm *m)
{
  m->program_counter++;
  uint16_t location = get_instruction(m);
  m->stack->base_pointer[m->stack->base_index + location] = pop(m->stack);
  m->program_counter++;
}

void call_iinc(ijvm *m)
{
  m->program_counter++;
  uint8_t location = get_instruction(m);
  m->program_counter++;
  int8_t signed_value = get_instruction(m);
  m->stack->base_pointer[m->stack->base_index + location] += signed_value;
  m->program_counter++;
}

void call_wide(ijvm *m)
{
  uint16_t location;

  m->program_counter++;
  switch (get_instruction(m))
  {
  case 0x15: // ILOAD
    location = get_short(m);
    m->program_counter += 2;
    push(m->stack, get_local_variable(m, location));
    m->program_counter++;
    break;
  case 0x36: // ISTORE
    location = (uint16_t)get_short(m);
    m->program_counter += 2;
    m->stack->base_pointer[m->stack->base_index + location] = pop(m->stack);
    m->program_counter++;
    break;
  case 0x84: // IINC
    location = (uint16_t)get_short(m);
    m->program_counter += 3;
    int8_t signed_value = get_instruction(m);
    m->stack->base_pointer[m->stack->base_index + location] += signed_value;
    m->program_counter++;
    break;
  }
}

void call_invokevirtual(ijvm *m)
{
  int old_program_counter = m->program_counter + 3;
  uint16_t location = get_short(m);       // fetch objref pos
  word_t var = get_constant(m, location); // fetch program_counter pointed to by objref

  m->program_counter = var - 1;

  uint16_t arg_count = (uint16_t)get_short(m); // no. of arguments (including objref)
  m->program_counter += 2;
  uint16_t lv_count = (uint16_t)get_short(m); // no. of local variables
  m->program_counter += 2;

  for (int i = 0; i < lv_count; i++)
  {
    push(m->stack, 0xCAFE);
  }

  push(m->stack, old_program_counter);
  push(m->stack, m->stack->base_index);

  m->stack->stack_pointer -= 2 + arg_count + lv_count;
  m->stack->base_index = m->stack->stack_pointer;

  push(m->stack, arg_count + lv_count);

  m->stack->stack_pointer += arg_count + lv_count + 1;

  m->program_counter++;
}

void call_ireturn(ijvm *m)
{
  word_t return_value = pop(m->stack);
  word_t LV = get_local_variable(m, 0);

  m->stack->stack_pointer = m->stack->base_index + LV + 2;

  word_t restored_base_index = pop(m->stack);
  m->program_counter = pop(m->stack);

  m->stack->stack_pointer = m->stack->base_index;
  m->stack->base_index = restored_base_index;

  push(m->stack, return_value);
}

void call_tailcall(ijvm *m)
{
  uint16_t location = get_short(m);       // fetch objref pos
  word_t var = get_constant(m, location); // fetch program_counter pointed to by objref

  unsigned int LV = m->stack->base_pointer[m->stack->base_index];

  word_t old_program_counter = m->stack->base_pointer[m->stack->base_index + LV];
  int tmp = m->stack->stack_pointer;
  m->stack->stack_pointer = m->stack->base_index + LV + 2;
  int prev_base_index = pop(m->stack);
  m->stack->stack_pointer = tmp;

  m->program_counter = var - 1;

  uint16_t arg_count = (uint16_t)get_short(m); // no. of arguments (including objref)
  m->program_counter += 2;
  uint16_t lv_count = (uint16_t)get_short(m); // no. of local variables
  m->program_counter += 3;

  for (int i = 0; i < lv_count; i++)
  {
    push(m->stack, 0xCAFE);
  }

  for (int i = arg_count + lv_count; i > 0; i--)
  {
    m->stack->base_pointer[m->stack->base_index + i - 1] = pop(m->stack);
  }

  m->stack->stack_pointer = m->stack->base_index + arg_count + lv_count;

  push(m->stack, old_program_counter);
  push(m->stack, prev_base_index);

  m->stack->base_pointer[m->stack->base_index] = arg_count + lv_count;
}

// Below: methods needed by bonus assignments, see ijvm.h
// You can leave these unimplemented if you are not doing these bonus
// assignments.

int get_call_stack_size(ijvm *m)
{
  // TODO: implement me if doing tail call bonus
  return m->stack->stack_pointer;
}

// Checks if reference is a freed heap array. Note that this assumes that
//
bool is_heap_freed(ijvm *m, word_t reference)
{
  // TODO: implement me if doing garbage collection bonus
  return 0;
}
