#include "cpu/exec/helper.h"



/*三次调用 mov-template.h的模板，每个模板内除了template-start.h和template-end.h都用了
make_instr_helper(i2r)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)

如果DATA_BYTE为1，以第一个为例子有如下的宏展开，是一个decode，execute函数
int instr_i2r_b(swaddr_t eip){
		return idex(eip,decode_i2r_b,do_excute);
	}
*/

#define DATA_BYTE 1
#include "mov-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "mov-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "mov-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

/*make_helper_v 以mov_i2r为例子作如下的展开
int mov_i2r_v(swaddr_t eip)
{
    return (ops_decoded.is_data_size_16?mov_i2r_w:mov_i2r_l)(eip);
}
根据指令执行时的情况，选择两个中的一个执行

*/
make_helper_v(mov_i2r)
make_helper_v(mov_i2rm)
make_helper_v(mov_r2rm)
make_helper_v(mov_rm2r)
make_helper_v(mov_a2moffs)
make_helper_v(mov_moffs2a)


