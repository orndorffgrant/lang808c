#ifndef IR_H
#define IR_H

// This enumerates all the types of IROps available
// They are grouped by how they use the result, arg1, and arg2 values
typedef enum _IROpCode {
    // result = arg1 OP arg2
    ir_add,
    ir_subtract,
    ir_shift_left,
    ir_bitwise_and,
    ir_equals,
    ir_less_than,

    // result = arg1
    ir_copy,

    // unconditional jump to target_label
    ir_goto,
    // if arg1 != 0, then jump to target_label
    ir_if,

    // arg1 used as param to upcoming function call
    ir_param,
    // call function in arg1
    ir_call
} IROpCode;
typedef enum _IRValueType {
    irv_function,
    irv_function_argument,
    irv_local_variable,
    irv_static_variable,
    irv_mmp_struct_item,
    irv_immediate,
    irv_temp
} IRValueType;
typedef struct _IRValue {
    IRValueType type;
    int func_index;
    int func_arg_index;
    int local_variable_index;
    int static_variable_index;
    int mmp_struct_item_index;
    int immediate_value;
    int temp_num;
} IRValue;
// An IROp is a Three Address Code Quadruple
typedef struct _IROp {
    int label;
    int target_label;
    IRValue result;
    IROpCode opcode;
    IRValue arg1;
    IRValue arg2;
} IROp;

void print_function_ir(IROp *code, int len);

#endif