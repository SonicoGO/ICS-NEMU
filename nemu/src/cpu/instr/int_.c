#include "cpu/intr.h"
#include "cpu/instr.h"

/*
Put the implementations of `int' instructions here.

Special note for `int': please use the instruction name `int_' instead of `int'.
*/

void raise_sw_intr(uint8_t intr_no);

make_instr_func(int_)
{
    opr_src.type = OPR_IMM;
    opr_src.sreg = SREG_CS;
    opr_src.data_size = 8;
    opr_src.addr = cpu.eip + 1;
    operand_read(&opr_src);
    raise_sw_intr(opr_src.val);
    
    return 0;
}
