#include <criterion/criterion.h>

/* common.h defines `is` as `==`; keep Criterion headers above project headers. */
#include "scanner.h"

static TokenType tok_type(const char *source)
{
	init_scanner(source);
	return scan_token().type;
}

Test(scanner, single_glyphs)
{
	cr_assert_eq(tok_type("("), TOKEN_LEFT_PAREN);
	cr_assert_eq(tok_type(")"), TOKEN_RIGHT_PAREN);
	cr_assert_eq(tok_type("{"), TOKEN_LEFT_BRACE);
	cr_assert_eq(tok_type("}"), TOKEN_RIGHT_BRACE);
	cr_assert_eq(tok_type(";"), TOKEN_SEMICOLON);
	cr_assert_eq(tok_type(","), TOKEN_COMMA);
	cr_assert_eq(tok_type("."), TOKEN_DOT);
	cr_assert_eq(tok_type("-"), TOKEN_MINUS);
	cr_assert_eq(tok_type("+"), TOKEN_PLUS);
	cr_assert_eq(tok_type("/"), TOKEN_SLASH);
	cr_assert_eq(tok_type("*"), TOKEN_STAR);
}

Test(scanner, two_char_operators)
{
	cr_assert_eq(tok_type("!="), TOKEN_BANG_EQUAL);
	cr_assert_eq(tok_type("=="), TOKEN_EQUAL_EQUAL);
	cr_assert_eq(tok_type("<="), TOKEN_LESS_EQUAL);
	cr_assert_eq(tok_type(">="), TOKEN_GREATER_EQUAL);
	cr_assert_eq(tok_type("**"), TOKEN_POW);
	cr_assert_eq(tok_type("++"), TOKEN_PLUS_PLUS);
}

Test(scanner, keywords_and_literals)
{
	cr_assert_eq(tok_type("and"), TOKEN_AND);
	cr_assert_eq(tok_type("class"), TOKEN_CLASS);
	cr_assert_eq(tok_type("false"), TOKEN_FALSE);
	cr_assert_eq(tok_type("true"), TOKEN_TRUE);
	cr_assert_eq(tok_type("nil"), TOKEN_NIL);
	cr_assert_eq(tok_type("var"), TOKEN_VAR);
	cr_assert_eq(tok_type("fun"), TOKEN_FUN);
	cr_assert_eq(tok_type("stdout"), TOKEN_PRINT_STDOUT);
	cr_assert_eq(tok_type("while"), TOKEN_WHILE);
	cr_assert_eq(tok_type("break"), TOKEN_BREAK);
	cr_assert_eq(tok_type("continue"), TOKEN_CONTINUE);

	init_scanner("answer");
	Token ident = scan_token();
	cr_assert_eq(ident.type, TOKEN_IDENTIFIER);
	cr_assert_eq(ident.length, (size_t)6);

	init_scanner("42.5");
	Token number = scan_token();
	cr_assert_eq(number.type, TOKEN_NUMBER);

	init_scanner("\"hi\"");
	Token string = scan_token();
	cr_assert_eq(string.type, TOKEN_STRING);

	init_scanner("");
	cr_assert_eq(scan_token().type, TOKEN_EOF);
}

Test(scanner, skips_whitespace_and_comments)
{
	init_scanner("  \t\n// comment\nvar");
	Token token = scan_token();
	cr_assert_eq(token.type, TOKEN_VAR);
	cr_assert_eq(token.line, (size_t)2);
}
