#include "common.h"
#include "cpu/reg.h"

#define IRQ_BASE 32
#define NO_INTR -1
#define MASK(n) (1 << (n))

typedef struct {
	uint8_t IRR;		/* interrupt request register */
	uint8_t ISR;		/* interrupt service register */
	uint8_t IMR;		/* interrupt mask register */
	int8_t highest_irq;
} i8259;

static i8259 master, slave;
static uint8_t intr_NO;

/* find first '1' */
/* this is a table containing the case information that a 8259 will encounter ,for example ,take from ir0 to ir7
as a state which range from 0 to 255.Now, if the 8259 state is 7 which means 00000111,zero bit is 1,so highest
privelige signal is 0 signal as the rule is smaller number higer priveilge. And we can now the only chances to
reply 7 siganl is state 10000000 which means no higher privilige than 7, therefore looking at table,ffo_table[128]
is 7.*/
static const char ffo_table[] = {
	-1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

/* i8259 internal */
static void do_i8259() {
	int8_t master_irq = master.highest_irq;
	if(master_irq == NO_INTR) {
		/* TODO: Uncomment the following line after the ``INTR'' member
		 * is added to the CPU_state structure.
		 */
		 cpu.INTR = false;
		//panic("uncomment the line above");
		return;
	}
	else if(master_irq == 2) {
		assert(slave.highest_irq != NO_INTR);
		master_irq = 8 + slave.highest_irq; //all slaved signal use a intr_NO
	}

	intr_NO = master_irq + IRQ_BASE;
	/* TODO: Uncomment the following line after the ``INTR'' member
	 * is added to the CPU_state structure.
	 */
	cpu.INTR = true;
	//panic("uncomment the line above");
}

/* device interface */
/*when there is a interupt signal , it will offer an interupt number. If the number is smaller than 8,master
/interupt request register get the offset of the interupt on the chip;but if it bigger or equal to 8,the signal
/will be considered as a slaved signal. when a slaved siganl is comming , we select highest previlige in slaved
chip,and slaved chip belongs to master 2.we transfer the information to do_i8259*/
void i8259_raise_intr(int n) {
	assert(n >= 0 && n < 16);
	if(n < 8) {
		master.IRR |= MASK(n);
	}
	else {
		n -= 8;
		slave.IRR |= MASK(n);
		master.IRR |= MASK(2);

		slave.highest_irq = ffo_table[slave.IRR & ~slave.IMR];
	}

	master.highest_irq = ffo_table[master.IRR & ~master.IMR];

	do_i8259();
}

/* CPU interface */
/*This is the function to reply interupt.  when nemu call the i8259_query_intr, first we get the bit offset of interupt on the chip
then we clear bit on the chip,then we call do_i8259 produce the new highest privilige signal.
*/
uint8_t i8259_query_intr() {
	return intr_NO;
}

void i8259_ack_intr() {
	if(intr_NO == NO_INTR) {
		return;
	}

	int n = intr_NO - IRQ_BASE;
	if(n < 8) {
		master.IRR &= ~MASK(n);
	}
	else {
		n -= 8;
		slave.IRR &= ~MASK(n);
		master.IRR &= ~MASK(2);

		slave.highest_irq = ffo_table[slave.IRR & ~slave.IMR];
	}

	master.highest_irq = ffo_table[master.IRR & ~master.IMR];

	do_i8259();
}
