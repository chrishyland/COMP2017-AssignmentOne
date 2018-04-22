/*************************************************************************************
*    21 April 2018
*    COMP2017 Assignment 1 - Virtual Stack
*    Charles Christopher Hyland
*    450411920
*    DESCRIPTION: EMULATING A VIRTUAL STACK.
*************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vstack.h"
#define BYTE unsigned char
#define BITS_IN_HALF_BYTE 4
#define BITS_IN_BYTES 8
#define MAX_NUM_FUNCTIONS 128
#define MAX_NUM_INSTRUCTIONS 256
#define END_PADDING 7
#define NO_FUNCTION_ID -1
#define DEFAULT_VALUE 0
#define NO_VALUE -1
#define TRUE 1
#define POINTER_TYPE 3

/*******************************************************************
* MAIN METHOD.
*******************************************************************/

int main(int n_args, char **vargs)
{
    /*******************************************************************
    * LOCATING THE FILE.
    *******************************************************************/

	char* filename = vargs[1];
    if(n_args != 2){
        printf("No argument was passed.\n");
        exit(0);
    }
    FILE *file_ptr = fopen(filename,"rb");
    fseek(file_ptr, 0, SEEK_END);
    size_t file_length = ftell(file_ptr);
    rewind(file_ptr);

    /*******************************************************************
    * GETTING BINARY REPRESENTATION OF FILE.
    *******************************************************************/

    int file_index_position = DEFAULT_VALUE;
    BYTE buffer;
    char *file = (char *) malloc(file_length * BITS_IN_BYTES + 1);
    char *file_one_byte;

    /*******************************************************************
    * PROCESS:
    *
    *                  [1] Load 1 byte of file into buffer.
    *                  [2] Convert buffer into binary representation.
    *                  [3] Store binary representaiton into our string.
    *******************************************************************/

    for (int i = 0; i < file_length; i++)
    {
        fread(&buffer, 1, 1, file_ptr);
        file_one_byte = convert_decimal_to_binary((int)buffer);
        memcpy(&file[file_index_position], file_one_byte, 8);
        file_index_position += 8;
        free(file_one_byte);
    }

    file[file_length * BITS_IN_BYTES] = '\0';

    /*******************************************************************
    * SETTING UP VIRTUAL STACK:
    *
    *                  [1] Allocate memory for virtual stack.
    *                  [2] Initialise stack and registers.
    *                  [3] Initialise function array.
    *                  [4] Allocate memory on heap for everything.
    *******************************************************************/

    struct VirtualStack *virtual_stack = (struct VirtualStack*) malloc(sizeof(struct VirtualStack));
    virtual_stack->stack = (BYTE *) calloc(MAX_NUM_FUNCTIONS, sizeof(BYTE));
    virtual_stack->num_functions_total = 0;
    for(int i=0;i<8;i++)
    {
       virtual_stack->memory_register[i] = 0;
    }
    virtual_stack->functions_array = (struct Function**) malloc(MAX_NUM_FUNCTIONS * sizeof(struct Function*));


    for(int i=0;i<MAX_NUM_FUNCTIONS;i++)
    {
        virtual_stack->functions_array[i] = (struct Function*) malloc(sizeof(struct Function));
        virtual_stack->functions_array[i]->instructions = (struct Instruction**) malloc(MAX_NUM_INSTRUCTIONS * sizeof(struct Instruction*));

        for(int j=0;j<MAX_NUM_INSTRUCTIONS;j++)
        {
            virtual_stack->functions_array[i]->instructions[j] = (struct Instruction*) malloc(sizeof(struct Instruction));
        }
    }

    /*******************************************************************
    * READING IN FILE TO LOAD INSTRUCTIONS INTO VIRTUAL STACK.
    *******************************************************************/

    int current_function_index = DEFAULT_VALUE;

    while(END_PADDING < file_index_position)
    {
        parse_function(file, &file_index_position, virtual_stack->functions_array[current_function_index],
                      (virtual_stack->functions_array[current_function_index]->instructions));
        virtual_stack->num_functions_total++;
        current_function_index++;
    }

    /*******************************************************************
    * INITIALISE MAIN METHOD ON STACK.
    *******************************************************************/

		// Where frame pointer and stack pointer will be.
    virtual_stack->stack[0] = DEFAULT_VALUE;
    virtual_stack->stack[2] = DEFAULT_VALUE;

		virtual_stack->frame_pointer = DEFAULT_VALUE;
    virtual_stack->program_counter = DEFAULT_VALUE;
    virtual_stack->return_value = DEFAULT_VALUE;

		// For current main function.
    int main_function_location = get_main_function_location(virtual_stack);
    virtual_stack->current_function_position = main_function_location;
    virtual_stack->stack_function_position = DEFAULT_VALUE;

		// Intialise stack of functions.
    for(int i = 0;i<128;i++)
    {
        if(i==0)
        {
					// MAIN FUNCTION ONLY.
          virtual_stack->function_stack[i] = DEFAULT_VALUE;
          continue;
        }
        virtual_stack->function_stack[i] = NO_VALUE;
    }

		// Stack pointer points to 2 + number of arguments in function.
		// Set the stack up with its stack pointer!
    (virtual_stack->stack_pointer) = (virtual_stack->functions_array[main_function_location]->num_arguments_function) + 2;
    virtual_stack->stack[1] = (virtual_stack->stack_pointer);

    /*******************************************************************
    * EXECUTE INSTRUCTIONS ONTO STACK.
    *                  [1] Check if Stack overflow.
    *                  [2] Parse information from current instruction in current function.
    *                  [3] Execute instruction onto stack.
    *******************************************************************/

    while(TRUE)
    {
        check_overflow(virtual_stack);

        int current_function_position = virtual_stack->current_function_position;
        int current_instruction_position = virtual_stack->program_counter;

        struct Instruction *current_instruction = virtual_stack->functions_array[current_function_position]->instructions[current_instruction_position];

        int *current_address_one = &current_instruction->address_one;
        int current_address_type_one = current_instruction->address_type_one;
        int *current_address_two = &current_instruction->address_two;
        int current_address_type_two = current_instruction->address_type_two;
        int operation_code = current_instruction->opcode;

        // Call functions depending on operation code.
        switch(operation_code){
            case 0:
                // 000 MOVE
                MOVE(virtual_stack, current_address_one, current_address_type_one, current_address_two, current_address_type_two);
                break;
            case 1:
                // 001 CALL
                CALL(virtual_stack, current_address_one, current_address_type_one, current_address_two, current_address_type_two);
                break;
            case 2:
                // 010 POP
                POP(virtual_stack, current_address_one, current_address_type_one);
                break;
            case 3:
                // 011 RETURN
                RETURN(virtual_stack);
                break;
            case 4:
                // 100 ADD
                ADD(virtual_stack, current_address_one, current_address_two);
                break;
            case 5:
                // 101 AND
                AND(virtual_stack, current_address_one, current_address_two);
                break;
            case 6:
                // 110 NOT
                NOT(virtual_stack, current_address_one);
                break;
            case 7:
                // 111 EQUAL
                EQUAL(virtual_stack, current_address_one);
                break;
        }
    }

    /*******************************************************************
    * FREE UP ALL MEMORY ON THE HEAP.
    *******************************************************************/

    free_all(virtual_stack, file);

		return 0;
}


/*******************************************************************
* FUNCTION IMPLEMENTATIONS FOR READING IN FILE.
*******************************************************************/

char *convert_decimal_to_binary(int decimal)
{
    /*************************************************************
    * Name: convert_decimal_to_binary
    * Description:  This will read in a decimal number and returns
    * the corresponding binary representation in string
    * through bitmasking.
    *************************************************************/

    int position_index = DEFAULT_VALUE;
    char *binary = (char*) malloc(BITS_IN_BYTES * sizeof(char) + 1);
    unsigned bit_mask_position = 1 << 7;
    while(bit_mask_position > DEFAULT_VALUE)
    {
        if((decimal & bit_mask_position) == 0)
        {
            binary[position_index] = '0';
        }else
        {
            binary[position_index] = '1';
        }
        position_index++;
        bit_mask_position = bit_mask_position >> 1;
    }
    binary[8] = '\0';
    return binary;
}

int convert_binary_to_decimal(char* binary, int number_bits)
{
    /*************************************************************
    * Name: convert_binary_to_decimal
    * Description:  This will read in a binary number and number
    * of bits. It will then output the corresponding decimal
    * representation in string through bitmasking.
    *************************************************************/

    int decimal = DEFAULT_VALUE, intermediary_value = DEFAULT_VALUE;
    int offset = 1;

    for(int i=number_bits-1; i>=0; i--)
    {
        intermediary_value = (int) (binary[i] - '0');
        decimal = decimal + (intermediary_value * offset);
        offset = 2 * offset;
    }
    return decimal;
}


void free_all(struct VirtualStack *virtual_stack, char *file)
{
    /*************************************************************
    * Name: free_all
    * Description:  This will free everything in virtual stack.
    *************************************************************/

    for(int i=0;i<MAX_NUM_FUNCTIONS;i++)
        {
            for(int j=0;j<MAX_NUM_INSTRUCTIONS;j++)
            {
                free(virtual_stack->functions_array[i]->instructions[j]);
            }
            free(virtual_stack->functions_array[i]->instructions);
            free(virtual_stack->functions_array[i]);
        }
    free(virtual_stack->functions_array);
    free(virtual_stack->stack);
    free(virtual_stack);
    free(file);
}

void parse_function(char *file, int *index_position, struct Function *function, struct Instruction **instruction)
{
    /*************************************************************
    * Name: parse_function
    * Description:  This will read in the instructions from binary
    * file and store them into function struct at the end.
    *************************************************************/

    // For Function Struct.
    int num_instructions = DEFAULT_VALUE;
    int current_function_id = NO_FUNCTION_ID;
    int num_arguments = DEFAULT_VALUE;

    // For Instruction Struct.
    int address_one = DEFAULT_VALUE;
    int address_type_two = DEFAULT_VALUE;
    int address_two = DEFAULT_VALUE;
    int address_type_one = DEFAULT_VALUE;
    int instruction_opcode = DEFAULT_VALUE;

    *index_position -= BITS_IN_BYTES;
    num_instructions = convert_binary_to_decimal(&file[*index_position], BITS_IN_BYTES);
    function->num_instructions = num_instructions;
    for(int i=num_instructions-1;i>=0;i--)
    {
        *index_position -= 3;
        instruction_opcode = convert_binary_to_decimal(&file[*index_position], 3);
        instruction[i]->opcode = instruction_opcode;

        switch(instruction_opcode)
        {

            case 3:
								// RETURN
                initialise_instruction(NO_VALUE, NO_VALUE, NO_VALUE, NO_VALUE, &instruction[i]);
                break;
            case 0:
            case 1:
            case 4:
            case 5:
							  // MOVE, CALL, ADD, AND.
                *index_position -= 2;
                address_type_one = convert_binary_to_decimal(&file[*index_position], 2);
                *index_position -= get_size_instruct_address(address_type_one);
                address_one = convert_binary_to_decimal(&file[*index_position], get_size_instruct_address(address_type_one));
                *index_position -= 2;
                address_type_two = convert_binary_to_decimal(&file[*index_position], 2);
                *index_position -= get_size_instruct_address(address_type_two);
                address_two = convert_binary_to_decimal(&file[*index_position], get_size_instruct_address(address_type_two));
                initialise_instruction(address_one, address_two, address_type_one, address_type_two, &instruction[i]);
                break;
            case 2:
            case 6:
            case 7:
							  // POP, NOT, EQUAL.
                *index_position -= 2;
                address_type_one = convert_binary_to_decimal(&file[*index_position], 2);
                *index_position -= get_size_instruct_address(address_type_one);
                address_one = convert_binary_to_decimal(&file[*index_position], get_size_instruct_address(address_type_one));
                initialise_instruction(address_one, -1, address_type_one, -1, &instruction[i]);
                break;
        }
    }

    *index_position -= BITS_IN_HALF_BYTE;
    num_arguments = convert_binary_to_decimal(&file[*index_position], BITS_IN_HALF_BYTE);
    *index_position -= BITS_IN_HALF_BYTE;
    current_function_id = convert_binary_to_decimal(&file[*index_position], BITS_IN_HALF_BYTE);
    function->function_id = current_function_id;
    function->num_arguments_function = num_arguments;
    function->instructions = instruction;
}


int get_size_instruct_address(int address_type)
{
    /*************************************************************
    * Name: get_size_instruct_address
    * Description:  Computes the size of the address.
    * Case [0]: Value
    * Case [1]: Register
    * Case [2]: Stack
    * Case [3]: Pointer
    *************************************************************/

    switch(address_type)
    {
        case 0: return 8;
        case 1: return 3;
        case 2: return 7;
        case 3: return 7;
    }
    return DEFAULT_VALUE;
}

void initialise_instruction(int add_one, int add_two, int address_type_one, int address_type_two, struct Instruction **instruction)
{
    /*************************************************************
    * Name: initialise_instruction
    * Description:  Initialises the instruction struct.
    * Case [0]: Value
    * Case [1]: Register
    * Case [2]: Stack
    * Case [3]: Pointer
    *************************************************************/

    (*instruction)->address_one = add_one;
    (*instruction)->address_type_one = address_type_one;
    (*instruction)->address_two = add_two;
    (*instruction)->address_type_two = address_type_two;
}

int get_main_function_location(struct VirtualStack *virtual_stack)
{
    /*************************************************************
    * Name: get_main_function_location
    * Description:  Returns the index in which main function exists
    * on virtual stack's array of functions.
    *************************************************************/

    for(int i=0;i<virtual_stack->num_functions_total;i++)
    {
        if(virtual_stack->functions_array[i]->function_id == 0)
        {
            return i;
        }
    }
    return NO_VALUE;
}

int get_function_location(int locate, struct VirtualStack *virtual_stack)
{
    /*************************************************************
    * Name: get_function_location
    * Description:  Returns the index in which function exists on
    * virtual stack's array of functions.
    *************************************************************/

    for(int i=0;i<virtual_stack->num_functions_total;i++)
    {
        if(virtual_stack->functions_array[i]->function_id == locate)
        {
            return i;
        }
    }
    return NO_VALUE;
}


/*************************************************************************************
* FUNCTION IMPLEMENTATIONS FOR EXECUTING INSTRUCTIONS.
*************************************************************************************/


void MOVE(struct VirtualStack *virtual_stack, int *address_one, int address_type_one, int *address_two, int address_type_two)
{
    /*************************************************************
    * Name: MOVE
    * Description:  MOVE value from one memory location to another.
    *
    * Check the types of addresses. THen get the addresses based
    * on possible pairings of memory addresses.
    *************************************************************/

    int frame_pointer = virtual_stack->frame_pointer;

		if(*address_one == 1 && address_type_one == 3)
		{
			// Pushing on top of stack.
			virtual_stack->stack_pointer += 1;
			virtual_stack->stack[frame_pointer + 1] += 1;
			check_overflow(virtual_stack);
			switch(address_type_two)
			{
				case 0:
					virtual_stack->stack[virtual_stack->stack_pointer] = *address_two;
					break;
				case 1:
				  virtual_stack->stack[virtual_stack->stack_pointer] = virtual_stack->memory_register[*address_two];
					break;
				case 2:
					virtual_stack->stack[virtual_stack->stack_pointer] = virtual_stack->stack[*address_two + frame_pointer];
					break;
			}
			increment_pointer_count(virtual_stack);
			return;
		}

    if(address_type_two == POINTER_TYPE)
    {
        // POINTER TYPE.
        *address_two = virtual_stack->stack[frame_pointer + *address_two];
        address_type_two = 2;
    }

    if(address_type_one == POINTER_TYPE)
    {
        // POINTER TYPE.
        *address_one = virtual_stack->stack[frame_pointer + *address_one];
        address_type_one = 2;
    }

    if(address_type_one == 2 && *address_one == (frame_pointer + 2))
    {
        // PROGRAM COUNTER.
        switch(address_type_two)
        {
            case 0:
                // VALUE
                virtual_stack->program_counter = *address_two;
                break;
            case 1:
                // REGISTER ADDRESS.
                virtual_stack->program_counter = virtual_stack->memory_register[*address_two];
                break;
            case 2:
                // STACK ADDRESS.
                virtual_stack->program_counter = (BYTE) virtual_stack->stack[frame_pointer + *address_two];
                break;
        }
    }

    switch(address_type_two){
        case 0:
            // ADDRESS TYPE 2 VALUE.
						switch(address_type_one)
						{
							case 1:
								// REGISTER.
								virtual_stack->memory_register[*address_one] = *address_two;
								break;
							case 2:
								// STACK.
								virtual_stack->stack[*address_one + frame_pointer] =  (BYTE) *address_two;
								break;
						}
						break;

        case 1:
            // ADDRESS TYPE 2 REGISTER.
						switch(address_type_one)
						{
							case 1:
							// REGISTER.
							virtual_stack->memory_register[*address_one] = virtual_stack->memory_register[*address_two];
							break;
						case 2:
							// STACK.
							virtual_stack->stack[*address_one + frame_pointer] = (BYTE) virtual_stack->memory_register[*address_two];
							break;
					 }
					break;

				case 2:
            // ADDRESS TYPE 2 STACK ADDRESS.
						switch(address_type_one)
						{
							case 1:
								virtual_stack->memory_register[*address_one] = virtual_stack->stack[*address_two + frame_pointer];
								break;
							case 2:
								virtual_stack->stack[*address_one + frame_pointer] = (BYTE) virtual_stack->stack[*address_two + frame_pointer];
								break;
						}
						break;
    }
    increment_pointer_count(virtual_stack);
}

void CALL(struct VirtualStack *virtual_stack, int *address_one, int address_type_one, int *address_two, int address_type_two)
{
    /*************************************************************
    * Name: CALL
    * Description: CALL another function. We will be given a function
    * id and stack address in which to play arguments into.
    *
    * [1] First we need to store current frame pointer.
    * [2] Update frame pointer to point to address above stack pointer.
    * [3] MOVE value within previous frame pointer into new location
    * on stack for frame pointer.
    * [4] Set value of virtual stack to be 2 addresses above new frame pointer.
    * [5] Update program counter for function caller's.
    * [6] Initialise program counter to 0 for new subroutine called.
    * [7] In the new subroutine, push the address of the first instruction on stack.
    * [8] Push arguments onto the stack in new subroutine.
    * [9] Update current function and current position.
    * [10] Update stack pointer.
    *************************************************************/

    check_overflow(virtual_stack);

    if(address_type_one == 3)
    {
        //Pointer. Need to dereference.
        *address_one = virtual_stack->stack[virtual_stack->frame_pointer + *address_one];
    }

    /*************************************************************
    * SAVE AND UPDATE FRAME POINTERS. [1]-[3]
    *************************************************************/
    virtual_stack->previous_frame_pointer = virtual_stack->frame_pointer;
    int previous_frame_pointer = virtual_stack->previous_frame_pointer;
    virtual_stack->frame_pointer = virtual_stack->stack_pointer+1;
    virtual_stack->stack[virtual_stack->frame_pointer] = virtual_stack->stack[previous_frame_pointer];

    /*************************************************************
    * UPDATE VALUES OF FRAME POINTER AND PROGRAM COUNT. [4]-7
    *************************************************************/
    virtual_stack->stack[(virtual_stack->frame_pointer) + 2] = 0;
    virtual_stack->stack[previous_frame_pointer + 2]++;
    virtual_stack->program_counter = 0;

    /*************************************************************
    * PUSH ARGUMENTS ONTO STACK FRAME. [8]
    *************************************************************/

    int function_location = get_function_location(*address_two, virtual_stack);
    int number_of_args = virtual_stack->functions_array[function_location]->num_arguments_function;

    for(int i=0; i<number_of_args; i++)
    {
        virtual_stack->stack[(virtual_stack->frame_pointer) + i + 3] = virtual_stack->stack[(virtual_stack->previous_frame_pointer) + i + *address_one];
    }

    /*************************************************************
    * UPDATE POSITION, FUNCTION, AND STACK. [9]
    *************************************************************/

    virtual_stack->current_function_position = function_location;
    virtual_stack->stack_function_position++;
    virtual_stack->function_stack[virtual_stack->stack_function_position] = *address_two;

    /*************************************************************
    * UPDATE STACK POINTER. [10]
    *************************************************************/

    virtual_stack->stack_pointer = virtual_stack->frame_pointer+number_of_args+2;
    virtual_stack->stack[virtual_stack->frame_pointer+1] = virtual_stack->stack_pointer;

    empty_registers(virtual_stack);
}

void POP(struct VirtualStack *virtual_stack, int *address_one, int address_type_one)
{
    /*************************************************************
    * Name: POP
    * Description:  POP from stack and store in virtual stack struct.
    * Virtual stack struct keeps track of latest return value from
    * functions.
    *************************************************************/

		if(address_type_one == 3)
		{
			*address_one = virtual_stack->stack[virtual_stack->frame_pointer + *address_one];

		}
		int value = virtual_stack->stack[virtual_stack->frame_pointer + *address_one];
		virtual_stack->return_value = value;
    increment_pointer_count(virtual_stack);
}

void RETURN(struct VirtualStack *virtual_stack)
{
    /*************************************************************
    * Name: RETURN
    * Description:  Check if main function, if yes, then print
    * return value and terminate program.
    *************************************************************/

    is_main(virtual_stack);
		int current_return_value = virtual_stack->return_value;
		int previous_frame = virtual_stack->previous_frame_pointer;
    virtual_stack->stack[virtual_stack->frame_pointer] = current_return_value;
		virtual_stack->stack_pointer = (virtual_stack->frame_pointer) - 1;

		int stack_index = (virtual_stack->frame_pointer)+1;
		int stack_end = (virtual_stack->stack_pointer)+1;
		for(int i = stack_index; i < stack_end; i++)
		{
			virtual_stack->stack[i] = 0;
		}

		virtual_stack->frame_pointer = previous_frame;
		virtual_stack->function_stack[virtual_stack->stack_function_position] = -1;
		virtual_stack->stack_function_position--;

		empty_registers(virtual_stack);

		int previous_function_position = get_function_location(virtual_stack->function_stack[virtual_stack->stack_function_position], virtual_stack);
		virtual_stack->current_function_position = previous_function_position;

		virtual_stack->program_counter = virtual_stack->stack[virtual_stack->frame_pointer + 2];

		increment_pointer_count(virtual_stack);
}

void ADD(struct VirtualStack *virtual_stack, int *address_one, int *address_two)
{
    /*************************************************************
    * Name: ADD
    * Description:  Takes 2 REGISTER addresses and ADD their
    * address values. Store results in first register.
    *************************************************************/

    int result =  virtual_stack->memory_register[*address_one] + virtual_stack->memory_register[*address_two];
    virtual_stack->memory_register[*address_two] = result;
    increment_pointer_count(virtual_stack);
}


void AND(struct VirtualStack *virtual_stack, int *address_one, int *address_two)
{
    /*************************************************************
    * Name: AND
    * Description:  Takes 2 REGISTER addresses and perform AND on
    * their address values. Store result in first register.
    *************************************************************/

		int result =  virtual_stack->memory_register[*address_one] & virtual_stack->memory_register[*address_two];
    virtual_stack->memory_register[*address_two] = result;
    increment_pointer_count(virtual_stack);
}

void NOT(struct VirtualStack *virtual_stack, int *register_address)
{
    /*************************************************************
    * Name: NOT
    * Description:  Take a REGISTER address and perform bitwise.
    * NOT operator and update accordingly.
    *************************************************************/

    virtual_stack->memory_register[*register_address] = ~(virtual_stack->memory_register[*register_address]);
    increment_pointer_count(virtual_stack);
}

void EQUAL(struct VirtualStack *virtual_stack, int *register_address)
{
    /*************************************************************
    * Name: EQUAL
    * Description:  Take a REGISTER address and see if equals 0.
    * Update the register address accordingly.
    *************************************************************/

    if(virtual_stack->memory_register[*register_address] == FALSE)
    {
        virtual_stack->memory_register[*register_address] = TRUE;
    }
    else{
        virtual_stack->memory_register[*register_address] = FALSE;
    }
		increment_pointer_count(virtual_stack);
}

/*************************************************************************************
* OPCODE INSTRUCTIONS HELPER FUNCTIONS.
*************************************************************************************/

void check_overflow(struct VirtualStack *virtual_stack)
{

    /*************************************************************
    * Name: check_overflow
    * Description:  Check and exit program if overflow.
    *************************************************************/

    if(((virtual_stack->stack_pointer) + (virtual_stack->frame_pointer)) >= 128)
    {
        printf("Stack Overflow!\n");
        exit(0);
    }

    if(virtual_stack->program_counter >= 128)
    {
        printf("Stack Overflow!\n");
        exit(0);
    }
}

void increment_pointer_count(struct VirtualStack *virtual_stack)
{
    /*************************************************************
    * Name: increment_pointer_count
    * Description:  Update program counter in struct and also on stack.
    *************************************************************/

    int location = virtual_stack->frame_pointer;
    virtual_stack->program_counter++;
    virtual_stack->stack[location+2]++;
		check_overflow(virtual_stack);
    return;
}


int check_is_register(int address_type)
{
    /*************************************************************
    * Name: check_is_register
    * Description:  Check if address type is register (3 bits).
    *************************************************************/

    if(address_type == 3) return TRUE;
    return FALSE;
}

int is_register(int address)
{
    /*************************************************************
    * Name: is_register
    * Description: Check if register or not.
    *************************************************************/

		if(address == 1) return 1;
    return 0;
}

void is_main(struct VirtualStack *virtual_stack)
{
    /*************************************************************
    * Name: is_main
    * Description:  Check if current function is main function.
    *************************************************************/

    if(virtual_stack->stack_function_position == 0){
        printf("%d\n", virtual_stack->return_value);
        exit(0);
    }
    return;
}




void empty_registers(struct VirtualStack *virtual_stack)
{
    /*************************************************************
    * Name: empty_registers
    * Description: Clear out registers since stack frame popped.
    *************************************************************/

		for(int i = 0; i<8; i++)
    {
        virtual_stack->memory_register[i] = 0;
    }
    return;
}
