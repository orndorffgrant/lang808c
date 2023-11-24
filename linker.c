#include "linker.h"
#include "armv6m.h"

void add16(uint8_t *dest, int curr_offset, uint16_t data) {
    for (int i = 0; i < 2; i++) {
        dest[curr_offset + i] = (uint8_t)(data >> (8 * i));
    }
}
void add32(uint8_t *dest, int curr_offset, uint32_t data) {
    for (int i = 0; i < 4; i++) {
        dest[curr_offset + i] = (uint8_t)(data >> (8 * i));
    }
}
int link(SymbolTable *symbols, MachineCode *code, uint8_t *dest) {
    int len = 0;
    // for all code in each function, assign address
    for (int i = 0; i < symbols->functions_num; i++) {
        for (int j = 0; j < code->functions[i].len; j++) {
            code->functions[i].ops[j].address = len;
            len += 2; // every instruction is two bytes;
        }
    }
    // for all code in each function, find BLs and fill in
    for (int i = 0; i < symbols->functions_num; i++) {
        for (int j = 0; j < code->functions[i].len; j++) {
            ARMv6Op *op = &code->functions[i].ops[j];
            if (op->target_function) {
                int curr_address = op->address;
                int target_address = code->functions[op->target_function].ops[0].address;
                int offset = (target_address - curr_address) - 4; // TODO double check -4
                //printf("offset: %d\n", offset);
                //printf("offset: %x\n", offset);
                int s = 0;
                int j1 = 0;
                int j2 = 0;
                if (offset < 0) {
                    s = 1;
                    j1 = 1;
                    j2 = 1;
                }
                //printf("s: %d, j1: %d, j2: %d\n", s, j1, j2);
                //printf("one: %x\n", ((uint32_t)offset) & 0b1111111111000000000000);
                uint16_t imm10 = (((uint32_t)offset) & 0b1111111111000000000000) >> 12;
                //printf("imm10: %x\n", imm10);
                uint16_t imm11 = (((uint32_t)offset) & 0b111111111110) >> 1;
                //printf("imm11: %x\n", imm11);
                op->code |= (s << 10) | imm10;
                ARMv6Op *next_op = &code->functions[i].ops[j+1];
                next_op->code |= (j1 << 13) | (j2 << 11) | (imm11);
            }
        }
    }

    // construct empty vector table, add to dest
    int curr_offset = 0;
    uint32_t reset_fn = 0xB1;

    add32(dest, curr_offset, 0x20008000); // stack pointer
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // reset
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // nmi
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // hard fault
    curr_offset += 4;
    add32(dest, curr_offset, 0); // reserved 1
    curr_offset += 4;
    add32(dest, curr_offset, 0); // reserved 2
    curr_offset += 4;
    add32(dest, curr_offset, 0); // reserved 3
    curr_offset += 4;
    add32(dest, curr_offset, 0); // reserved 4
    curr_offset += 4;
    add32(dest, curr_offset, 0); // reserved 5
    curr_offset += 4;
    add32(dest, curr_offset, 0); // reserved 6
    curr_offset += 4;
    add32(dest, curr_offset, 0); // reserved 7
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // svc
    curr_offset += 4;
    add32(dest, curr_offset, 0); // reserved 8
    curr_offset += 4;
    add32(dest, curr_offset, 0); // reserved 9
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // pendsv
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // systick
    curr_offset += 4;
    // end core arm vector table
    // begin samd21-specific vector table
    add32(dest, curr_offset, reset_fn); // power_manager_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // system_control_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // watchdog_timer_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // realtime_counter_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // external_interrupt_controller_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // nonvolatile_mem_controller_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // dma_controller_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // usb_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // event_system_interface_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // sercom0_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // sercom1_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // sercom2_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // sercom3_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // sercom4_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // sercom5_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // timer_counter_control_0_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // timer_counter_control_1_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // timer_counter_control_2_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // timer_counter_3_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // timer_counter_4_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // timer_counter_5_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // timer_counter_6_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // timer_counter_7_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // adc_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // analog_comparator_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // dac_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // peripheral_touch_controller_handler
    curr_offset += 4;
    add32(dest, curr_offset, reset_fn); // i2s_handler
    curr_offset += 4;

    // for all code in each function, put in dest
    for (int i = 0; i < symbols->functions_num; i++) {
        // is it an interrupt handler?
        for (int k = 0; k < symbols->interrupt_handlers_num; k++) {
            if (symbols->interrupt_handlers[k].func_index == i) {
                // then we put it in the vector table
                // curr_offset is the address of the function
                // calculate position of vector table entry
                int position = (16 * 4) + (symbols->interrupt_handlers[k].interrupt_number * 4);
                add32(dest, position, curr_offset + 1); // add one for some reason
            }
        }
        for (int j = 0; j < code->functions[i].len; j++) {
            add16(dest, curr_offset, code->functions[i].ops[j].code);
            curr_offset += 2;
        }
    }

    return curr_offset;
}
void print_hex(uint8_t *code, int len) {
    int num_full_rows = len / 0x10;
    int len_last_row = len % 0x10;
    // all full rows
    for (int i = 0; i < (num_full_rows + 1); i++) {
        int8_t sum = 0;
        printf(":"); // start char

        printf("10"); // byte count
        sum += 0x10;

        int curr_address = i * 0x10;
        printf("%04x", curr_address); // address
        sum += (curr_address >> 8);
        sum += (curr_address & 0xff);

        printf("00"); // record type 00 is data
        sum += 0;

        if (i < num_full_rows) {
            for (int j = 0; j < 0x10; j++) {
                printf("%02x", code[(i * 0x10) + j]);
                sum += code[(i * 0x10) + j];
            }
        } else {
            // the last non-full row, pad with zeroes
            int j;
            for (j = 0; j < len_last_row; j++) {
                printf("%02x", code[(i * 0x10) + j]);
                sum += code[(i * 0x10) + j];
            }
            for (; j < 0x10; j++) {
                printf("%02x", 0);
                sum += 0;
            }
        }
        printf("%02x", (uint8_t)-sum); // checksum
        printf("\n");
    }
    // start linear address - hardcoded to reset vector
    printf(":04000005000000B146\n");
    // end of file
    printf(":00000001FF\n");
}