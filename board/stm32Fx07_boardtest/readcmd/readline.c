#include <stdio.h>
#include <string.h>
#include "Readline.h"

#define CFG_CBSIZE 200
#define CFG_PROMPT		"emy@boot #"
#define CONFIG_SYS_MAXARGS 16

#define isblank(c)	(c == ' ' || c == '\t')



char console_buffer[CFG_CBSIZE];


struct cmd_tbl_s {
	char		*name;		/* Command Name			*/
	int		maxargs;	/* maximum number of arguments	*/
	int		repeatable;	/* autorepeat allowed?		*/
					/* Implementation function	*/
	int		(*cmd)(struct cmd_tbl_s *, int, int, char *[]);
	char		*usage;		/* Usage message	(short)	*/
	char		*help;		/* Help  message	(long)	*/

};

typedef struct cmd_tbl_s	cmd_tbl_t;


#include "cmd_array.c"


void print_invalide_cmd(void)
{
//	cmd_tbl_t *cmdtp;
	int tablen;

	for (tablen = 0;tablen < sizeof(gCmd_array)/sizeof(gCmd_array[0]);
	     tablen++) {
		 s_putstring(gCmd_array[tablen].name);
		 s_putstring("\r\n");
	}
}
cmd_tbl_t *find_cmd (const char *cmd)
{
	cmd_tbl_t *cmdtp;
	int tablen;

	if (!cmd)
		return NULL;

	for (cmdtp = &gCmd_array[0];tablen < sizeof(gCmd_array)/sizeof(gCmd_array[0]);
	     tablen++) {
		if (strcmp (cmd, cmdtp->name) == 0) {
				return cmdtp;	/* full match */
		}
		cmdtp++;
	}
	return NULL;	/* not found or ambiguous command */

}


int parse_line (char *line, char *argv[])
{
	int nargs = 0;


	while (nargs < CONFIG_SYS_MAXARGS) {

		/* skip any white space */
		while (isblank(*line))
			++line;

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;

			return (nargs);
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		while (*line && !isblank(*line))
			++line;

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			return (nargs);
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	s_putstring ("** Too many args **\r\n");

	return (nargs);
}


int run_command (const char *cmd, int flag)
{
	cmd_tbl_t *cmdtp;
	char cmdbuf[CFG_CBSIZE];	/* working copy of cmd		*/




	char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	int argc;

	int rc = 0;


	if (!cmd || !*cmd) {
		return -1;	/* empty command */
	}
	if(*cmd == '?'){
		print_invalide_cmd();
		return 0;
	}

	if (strlen(cmd) >= CFG_CBSIZE) {
		s_putstring ("## Command too long!\r\n");
		return -1;
	}

	strcpy (cmdbuf, cmd);

	/* Extract arguments */
	if ((argc = parse_line (cmdbuf, argv)) == 0) {
		rc = -1;	/* no command at all */
		return -1;
	}

	/* Look up command in command table */
	if ((cmdtp = find_cmd(argv[0])) == NULL) {
		s_putstring ("Unknown command - try 'help' \r\n");
		rc = -1;	/* give up after bad command */
		return -1;
	}

	/* OK - call function to do the command */
	if ((cmdtp->cmd) (cmdtp, flag, argc, argv) != 0) {
		rc = -1;
		return rc;
	}


	return rc;
}
static char * delete_char (char *p,int * np)
{

	if (*np == 0) {
		return (p);
	}

	*(p-1) = 0;
	p--;
	s_putstring ("\b \b");
	//s_putstring (console_buffer);

	(*np)--;
	return (p);
}


int readline (void )
{

	char   *p = console_buffer;
	int	n = 0;				/* buffer index		*/
	char	c;
	char prompt[] = CFG_PROMPT;
	char repeat_key[3];

	/* print prompt */
	if (prompt) {
		s_putstring(prompt);
	}
	for (;;) {

		c = s_getchar();

		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r':				/* Enter		*/
		case '\n':
			*p = '\0';
			s_putstring ("\r\n");
			return (p - console_buffer);

		case '\0':				/* nul			*/
			continue;

		case 0x08:				/* ^H  - backspace	*/
		case 0x7F:				/* DEL - backspace	*/
			p=delete_char(p,&n);
			continue;

		/* repeat key: 0x1B,0x5B,0x41 */
		case 0x1B:
			repeat_key[0] = 0x1B;
			continue;
		case 0x5B:
			repeat_key[1] = 0x5B;
			continue;
		case 0x41:
			if( (repeat_key[1] == 0x5B)&&(repeat_key[0] == 0x1B) ){
				//s_putstring(CFG_PROMPT);
				s_putstring(console_buffer);
				p = console_buffer + strlen(console_buffer);
			}
			continue;

		default:
			/*
			 * Must be a normal character then
			 */
			if (n < CFG_CBSIZE-2) {
				s_putchar (c);
				
				*p++ = c;
				++n;
			} else {			/* Buffer full		*/
				s_putstring ("Buffer full\n");
			}
		}
	}

}

void main_loop(void )
{
	int len;
	for(;;){
		len = readline();
		if(len > 0){
			run_command(console_buffer,1);
		}
	}
}
