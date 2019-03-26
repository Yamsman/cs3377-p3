#ifndef LEXER_H
#define LEXER_H

enum TOKEN_TYPES {
	TOK_STRING,
	TOK_PIPE,
	TOK_SEMI,
	TOK_LRDIR,
	TOK_RRDIR,
	TOK_APPEND,
	TOK_END
};

struct LEXER {
	int c_tok;
	char *tok_str;
	char *buf;
	char *pos;
};

void lexer_init(struct LEXER *lx, char *buf);
void lexer_adv(struct LEXER *lx);

#endif
