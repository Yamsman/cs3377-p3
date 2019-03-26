#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

void lexer_init(struct LEXER *lx, char *buf) {
	lx->buf = buf;
	lx->pos = buf;
	lexer_adv(lx);
	return;
}

void lexer_adv(struct LEXER *lx) {
	//Skip whitespace
	while (isspace(*lx->pos))
		lx->pos++;

	//Identify the token
	char *begin = lx->pos;
	char *str;
	char delim;
	int len;
	switch (*lx->pos) {
		case '|': lx->c_tok = TOK_PIPE;		break;
		case ';': lx->c_tok = TOK_SEMI;		break;
		case '<': lx->c_tok = TOK_LRDIR;	break;
		case '>':
			//Check for append token
			if (*(lx->pos+1) == '>') {
				lx->c_tok = TOK_APPEND;
				lx->pos++;
				break;
			}
			lx->c_tok = TOK_RRDIR;
			break;
		case '"':
		case '\'':
			//Get quotation bounds
			delim = (*lx->pos == '"') ? '"' : '\'';
			lx->pos++;
			while (*lx->pos != delim) {
				if (*lx->pos == '\0') {
					lx->c_tok = TOK_END;
					puts("Error: unterminated quotation");
					return;
				}
				lx->pos++;
			}
			
			//Copy to buffer
			len = lx->pos - begin - 1;
			str = malloc(len+1);
			memcpy(str, begin+1, len);
			str[len] = '\0';

			lx->c_tok = TOK_STRING;
			lx->tok_str = str;
			break;
		default:
			//Skip whitespace
			while (!isspace(*lx->pos) && !strchr("|;><", *lx->pos) &&
				*lx->pos != '\0') {
				lx->pos++;
			}

			//Copy the string
			len = lx->pos - begin;
			str = malloc(len+1);
			memcpy(str, begin, len);
			str[len] = '\0';

			lx->c_tok = TOK_STRING;
			lx->tok_str = str;
			lx->pos--;
			break;
		case '\0': lx->c_tok = TOK_END;	return;
	}
	lx->pos++;
	return;
}
