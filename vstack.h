/*************************************************************************************
*    21 April 2018
*    COMP2017 Assignment 1 - Virtual Stack
*    Charles Christopher Hyland
*    450411920
*    DESCRIPTION: HEADER FILE FOR vstack.c
*************************************************************************************/

#ifndef VSTACK
#define VSTACK
#include <stdio.h>
#include <stdlib.h>
#define BYTE unsigned char
#define TRUE 1
#define FALSE 0
#define DEFAULT_VALUE 0
#define NO_VALUE -1
#define POINTER_TYPE 3


/*************************************************************************************
* STRUCT DECLARATIONS.
*************************************************************************************/


struct Instruction
{

  /*************************************************************************************
  * STORES ALL INFORMATION FOR A GIVEN INSTRUCTION.
  * ADDRESS TYPE:
  *              [0]: Value. 1 Byte Long.
  *              [1]: Register Address. 3 Bits Long.
  *              [2]: Stack Address. 7 Bits Long.
  *              [3]: Pointer to Stack Address. 7 Bits Long.
  *
  * OPCodes:
  *              [0]: MOVE.
  *              [1]: CALL.
  *              [2]: POP.
  *              [3]: RETURN.
  *              [4]: ADD.
  *              [5]: AND.
  *              [6]: NOT.
  *              [7]: EQUAL.
  *************************************************************************************/

    int address_one;
    int address_type_one;
    int address_two;
    int address_type_two;
    int opcode;
};

struct Function
{

  /*************************************************************************************
  * STORES ALL INFORMATION FOR A GIVEN INSTRUCTION
  * id: Function ID
  * num_arguments_function: Number of arguments
  * instructions: Array to store all the instructions for one function.
  * num_instructions: How many instructions in this function.
  *************************************************************************************/

	int function_id;
	int num_arguments_function;
  struct Instruction **instructions;
  int num_instructions;
};

struct VirtualStack
{

  /*************************************************************************************
  * CONTAINS ALL INFORMATION REGARDING VIRTUAL STACK.
  * stack: Actual stack itself.
  * program_counter: How many instructions executed in subroutine.
  * frame_pointer: Where does the current stack frame begin.
  * stack_pointer: Points to end of current stack frame.
  * num_functions_total: Number of functions for the whole program.
  * functions_array: Array containing all function structs.
  * current_function_position: The location of executing function in the functions array containing all functions.
  * previous_frame_pointer: Location of previous function in functions array that called current function.
  * functions_stack:  Array containing functions that are pushed onto the stack.
  * stack_function_position: Location of the function pushed on the stack in the array.
  * return_value: The return value of the final program.
  * memory_register: Register to store all the values.
  *************************************************************************************/

  BYTE *stack;
  int program_counter;
  int frame_pointer;
  int stack_pointer;
  int num_functions_total;
  struct Function **functions_array;
  int current_function_position;
  int previous_frame_pointer;
  int function_stack[128];
  int stack_function_position;
  int return_value;
  BYTE memory_register[8];
};

/*************************************************************************************
* FUNCTION DECLARATIONS for Virtual Stack Operations.
*************************************************************************************/

// READING IN FILES.
char *convert_decimal_to_binary(int decimal);
int convert_binary_to_decimal(char* binary, int num_bits);
void parse_function(char *file, int *index_position, struct Function *function, struct Instruction **instruction);
int get_size_instruct_address(int address_type);
void initialise_instruction(int address_one, int address_two, int address_type_one, int addres_type_two, struct Instruction **instruction);

// INITIALISING VIRTUAL STACK.
int get_function_location(int locate, struct VirtualStack *virtual_stack);
int get_main_function_location(struct VirtualStack *virtual_stack);


// RUNNING VIRTUAL STACK.
int get_function_location(int locate, struct VirtualStack *virtual_stack);


// END OF FILE ADMIN STUFF.
void free_all(struct VirtualStack *virtual_stack, char *file);

/*************************************************************************************
* OPCODE FUNCTIONS.
*************************************************************************************/
void MOVE(struct VirtualStack *virtual_stack, int *address_one, int address_type_one, int *address_two, int address_type_two);
void CALL(struct VirtualStack *virtual_stack, int *address_one, int address_type_one, int *address_two, int address_type_two);
void POP(struct VirtualStack *virtual_stack, int *address_one, int address_type_one);
void RETURN(struct VirtualStack *virtual_stack);
void ADD(struct VirtualStack *virtual_stack, int *address_one, int *address_two);
void AND(struct VirtualStack *virtual_stack, int *address_one, int *address_two);
void NOT(struct VirtualStack *virtual_stack, int *register_address);
void EQUAL(struct VirtualStack *virtual_stack, int *register_address);

/*************************************************************************************
* HELPER FUNCTIONS.
*************************************************************************************/
void increment_pointer_count(struct VirtualStack *virtual_stack);
void check_overflow(struct VirtualStack *virtual_stack);
int check_is_register(int address_type);
int is_register(int address);
void is_main(struct VirtualStack *virtual_stack);
void empty_registers(struct VirtualStack *virtual_stack);

#endif
