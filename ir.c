#include "common.h"
#include "ir.h"

void print_ir_value(IRValue *value) {
    switch (value->type) {
        case irv_mmp_struct_item:
            // TODO need symbols
            STRINGREF_TO_CSTR1(symbols->struct_items[value->mmp_struct_item_index].name, 512);
            break;
    }
}
void print_function_ir(IROp *code, int len) {
    switch (code->opcode) {
        case ir_add:
            print_ir_value(&code->result);
            printf(" = ");
            print_ir_value(&code->arg1);
            printf(" + ");
            print_ir_value(&code->arg2);
            printf("\n");
            break;
        case ir_copy:
            print_ir_value(&code->result);
            printf(" = ");
            print_ir_value(&code->arg1);
            printf("\n");
        default: PANIC("UNIDENTIFIED IR\n");
    }
}