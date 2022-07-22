#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <elf.h>

extern int findSym(char *name);

uint32_t look_up_symtab(char *sym, bool *success){return 0;}


enum {
	NOTYPE = 256, EQ, NEQ,HEX,NUM,REG,SYMB,LS,RS,NG,NL,AND,OR,DEREF

	/* TODO: Add more token types */

};


static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

        {" +",	NOTYPE},				// white space
	{"\\+", '+'},					// add
	{"\\-", '-'},					// sub
	{"==", EQ},					//equal
	{"!=",NEQ},					//unequal
	{"0[xX][0-9a-fA-F]+", HEX},			//hexidecimal	
	{"[0-9]+", NUM},				//number
	{"\\$e[a,d,c,b]x", REG},			//register
	{"\\$e[s,b]p", REG},
	{"\\$e[d,s]i", REG},
	{"\\$eip",REG},
	{"[a-zA-Z]+[a-zA-Z0-9_]*", SYMB},		//symbol
	{"\\*", '*'},					//mul or ref
	{"/", '/'},					//div
	{"%", '%'},					//mod
	{"\\(", '('},					//l_bracket
	{"\\)", ')'},					//r_bracket
	{"<<", LS},					//left shift
	{">>", RS},					//right shift
	{"<=", NG},					//not greater than
	{">=", NL},					//not less than
	{"<", '<'},					//less than
	{">", '>'},					//greater than
	{"&&",AND},					//AND
	{"\\|\\|",OR},					//OR
	{"&",'&'},					//and
	{"\\^",'^'},					//xor
	{"\\|",'|'},					//or
	{"!",'!'},					//non
	{"~",'~'}					//reverse

};


static struct priority
{
	int token_type;
	int prior;
} prior_arr[] = {
	{'+',4},
	{'-',4},
	{EQ,7},
	{NEQ,7},
	{'*',3},
	{'/',3},
	{'%',3},
	{LS,5},
	{RS,5},
	{NG,6},
	{NL,6},
	{'<',6},
	{'>',6},
	{AND,11},
	{OR,12},
	{'&',8},
	{'^',9},
	{'|',10},
	{'!',2},
	{'~',2},
	{DEREF,2}
};	


#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )
#define NR_PRIOR (sizeof(prior_arr) / sizeof(prior_arr[0]) )
#define MAX_PRIOR 15

static regex_t re[NR_REGEX];


/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				//Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array ``tokens''. For certain 
				 * types of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case NOTYPE:
						break;
					case HEX:
					case NUM:
					case REG:
					case SYMB:
						strncpy(tokens[nr_token].str,substr_start,substr_len);
						tokens[nr_token].str[substr_len]='\0';
					default: 
						tokens[nr_token].type = rules[i].token_type;
						nr_token++;
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

int find_prior(uint32_t token_type)
{
	int i;
	for(i=0;i<NR_PRIOR ;i++)
	{
		if(prior_arr[i].token_type == token_type)
			return prior_arr[i].prior;
	}
	return -1;
}

int check_parentheses(uint32_t p,uint32_t q)
{
	if(tokens[p].type == '(' && tokens[q].type == ')')
		return true;
	else
		return false;
}
uint32_t eval(int p,int q,bool* success) {
	//printf("%d - %d\n",p,q);
    if(p > q) {
        /* Bad expression */
		*success = false; 
		return 0;
    }
    else if(p == q) { 
        /* Single token.
         * For now this token should be a number. 
         * Return the value of the number.
         */
		uint32_t result;
		switch(tokens[p].type)
		{
			case NUM:
				sscanf(tokens[p].str,"%d",&result);
				return result;
			case REG:
				if(!strcmp(tokens[p].str,"$eax"))
					return cpu.eax;
				else if(!strcmp(tokens[p].str,"$ebx"))
					return cpu.ebx;
				else if(!strcmp(tokens[p].str,"$ecx"))
					return cpu.ecx;
				else if(!strcmp(tokens[p].str,"$edx"))
					return cpu.edx;
				else if(!strcmp(tokens[p].str,"$esp"))
					return cpu.esp;
				else if(!strcmp(tokens[p].str,"$esi"))
					return cpu.esi;
				else if(!strcmp(tokens[p].str,"$ebp"))
					return cpu.ebp;
				else if(!strcmp(tokens[p].str,"$edi"))
					return cpu.edi;
				else 
					return cpu.eip;
			case HEX:
				sscanf(tokens[p].str,"0x%x",&result);
				return result;
			case SYMB:
				result = findSym(tokens[p].str);
				return result;
			default:
				return look_up_symtab(tokens[p].str,success);
		}
	
    }
    else if(check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses. 
         * If that is the case, just throw away the parentheses.
         */
        return eval(p + 1, q - 1, success); 
    }
    else 
    {
	
		int prior = MAX_PRIOR,op = -1,find = 0;
		while(prior!=0 && !find)
		{
			int i;
			for(i = q; i>=p; i--)
			{
				int temp = find_prior(tokens[i].type);

				if(tokens[i].type == ')')
				{
					int j;
					for(j = i; j>=p; j--)
					{
						if(tokens[j].type == '(')
						{
							i = j;
							break;
						}
					}
				}
				else if(prior == temp)
				{
					find = 1;
					op = i;
					break;
				}
			}	
			prior--;
		}
       // op = the position of dominant operator in the token expression;
		int val1 = eval(p, op - 1,success);
        int val2 = eval(op + 1, q,success);

		//printf("get sub value\n");
        switch(tokens[op].type) 
		{
			case '+': return val1 + val2; break;
			case '-': return val1 - val2; break;
			case '*': return val1 * val2; break;
			case '/': return val1 / val2; break;
			case '%': return val1 % val2; break;
			case '&': return val1 & val2; break;
			case '^': return val1 ^ val2; break;
			case '|': return val1 | val2; break;
			case '~': return ~val2; break;
			case '!': return !val2; break;
			case '<': return val1 < val2; break;
			case '>': return val1 > val2; break;
			case LS: return val1 << val2; break;
			case RS: return val1 >> val2; break;
			case NG: return val1 <= val2; break;
			case NL: return val1 >= val2; break;
			case EQ: return val1 == val2; break;
			case NEQ: return val1 != val2; break;
			case AND: return val1 && val2; break;
			case OR: return val1 || val2; break;
			case DEREF: return *(hw_mem + val2); break;
			default: assert(0);
		}
    }
}

int operator_judge(uint32_t type)
{

	if(type == '+'|| type == '-' || type == EQ || type == NEQ || type == '*' ||type =='/' ||type == '%' \
		||type == LS ||type == RS ||type == NG ||type == NL ||type == '<' ||type == '>' ||type == AND || \
		type == OR ||type =='&' ||type == '^' || type =='|' ||type =='!'||type =='~')
		return true;
	else
		return false; 
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	/* TODO: Insert codes to evaluate the expression. */
	int i;	
	for(i = 0; i < nr_token; i++) 
	{
   		 if(tokens[i].type == '*' && (i == 0 || operator_judge(tokens[i - 1].type)  ) )
        		tokens[i].type = DEREF;
	}
	
	int result = eval(0,nr_token-1,success);
	return result;
}

