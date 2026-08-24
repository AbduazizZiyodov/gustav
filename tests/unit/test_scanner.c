#include <criterion/criterion.h>
#include <string.h>

#include "scanner.h"

static void expect_token(Token token, TokenType type, const char *lexeme)
{
	cr_assert_eq(token.type, type, "expected token type %d got %d (%s)", (int)type,
		     (int)token.type, TOKEN_TYPE_STRING[token.type]);
	if (lexeme != NULL) {
		size_t expected_len = strlen(lexeme);
		cr_assert_eq(token.length, expected_len, "lexeme length: expected %zu got %zu",
			     expected_len, token.length);
		cr_assert_eq(memcmp(token.start, lexeme, expected_len), 0,
			     "lexeme mismatch: expected '%s'", lexeme);
	}
}

Test(scanner, empty_source_is_eof)
{
	init_scanner("");
	expect_token(scan_token(), TOKEN_EOF, "");
}

Test(scanner, whitespace_and_line_comments_skipped)
{
	init_scanner("  \t// comment only\n  \r\n");
	expect_token(scan_token(), TOKEN_EOF, "");
}

Test(scanner, single_character_tokens)
{
	init_scanner("(){},.-;/+");
	expect_token(scan_token(), TOKEN_LEFT_PAREN, "(");
	expect_token(scan_token(), TOKEN_RIGHT_PAREN, ")");
	expect_token(scan_token(), TOKEN_LEFT_BRACE, "{");
	expect_token(scan_token(), TOKEN_RIGHT_BRACE, "}");
	expect_token(scan_token(), TOKEN_COMMA, ",");
	expect_token(scan_token(), TOKEN_DOT, ".");
	expect_token(scan_token(), TOKEN_MINUS, "-");
	expect_token(scan_token(), TOKEN_SEMICOLON, ";");
	expect_token(scan_token(), TOKEN_SLASH, "/");
	expect_token(scan_token(), TOKEN_PLUS, "+");
	expect_token(scan_token(), TOKEN_EOF, "");
}

Test(scanner, two_character_operators)
{
	init_scanner("!= == <= >= ++ **");
	expect_token(scan_token(), TOKEN_BANG_EQUAL, "!=");
	expect_token(scan_token(), TOKEN_EQUAL_EQUAL, "==");
	expect_token(scan_token(), TOKEN_LESS_EQUAL, "<=");
	expect_token(scan_token(), TOKEN_GREATER_EQUAL, ">=");
	expect_token(scan_token(), TOKEN_PLUS_PLUS, "++");
	expect_token(scan_token(), TOKEN_POW, "**");
	expect_token(scan_token(), TOKEN_EOF, "");
}

Test(scanner, single_character_operators_when_not_doubled)
{
	init_scanner("! = < > *");
	expect_token(scan_token(), TOKEN_BANG, "!");
	expect_token(scan_token(), TOKEN_EQUAL, "=");
	expect_token(scan_token(), TOKEN_LESS, "<");
	expect_token(scan_token(), TOKEN_GREATER, ">");
	expect_token(scan_token(), TOKEN_STAR, "*");
	expect_token(scan_token(), TOKEN_EOF, "");
}

Test(scanner, keywords_and_identifiers)
{
	init_scanner("and class else false for fun if loop nil or return stdout stderr "
		     "super this true var while break continue foo_bar x1");
	expect_token(scan_token(), TOKEN_AND, "and");
	expect_token(scan_token(), TOKEN_CLASS, "class");
	expect_token(scan_token(), TOKEN_ELSE, "else");
	expect_token(scan_token(), TOKEN_FALSE, "false");
	expect_token(scan_token(), TOKEN_FOR, "for");
	expect_token(scan_token(), TOKEN_FUN, "fun");
	expect_token(scan_token(), TOKEN_IF, "if");
	expect_token(scan_token(), TOKEN_LOOP, "loop");
	expect_token(scan_token(), TOKEN_NIL, "nil");
	expect_token(scan_token(), TOKEN_OR, "or");
	expect_token(scan_token(), TOKEN_RETURN, "return");
	expect_token(scan_token(), TOKEN_PRINT_STDOUT, "stdout");
	expect_token(scan_token(), TOKEN_PRINT_STDERR, "stderr");
	expect_token(scan_token(), TOKEN_SUPER, "super");
	expect_token(scan_token(), TOKEN_THIS, "this");
	expect_token(scan_token(), TOKEN_TRUE, "true");
	expect_token(scan_token(), TOKEN_VAR, "var");
	expect_token(scan_token(), TOKEN_WHILE, "while");
	expect_token(scan_token(), TOKEN_BREAK, "break");
	expect_token(scan_token(), TOKEN_CONTINUE, "continue");
	expect_token(scan_token(), TOKEN_IDENTIFIER, "foo_bar");
	expect_token(scan_token(), TOKEN_IDENTIFIER, "x1");
	expect_token(scan_token(), TOKEN_EOF, "");
}

Test(scanner, numbers)
{
	init_scanner("0 42 3.14 10.0");
	expect_token(scan_token(), TOKEN_NUMBER, "0");
	expect_token(scan_token(), TOKEN_NUMBER, "42");
	expect_token(scan_token(), TOKEN_NUMBER, "3.14");
	expect_token(scan_token(), TOKEN_NUMBER, "10.0");
	expect_token(scan_token(), TOKEN_EOF, "");
}

Test(scanner, strings_and_unterminated)
{
	init_scanner("\"hi\" \"multi\nline\"");
	Token t = scan_token();
	expect_token(t, TOKEN_STRING, "\"hi\"");
	cr_assert_eq(t.line, 1);

	t = scan_token();
	expect_token(t, TOKEN_STRING, "\"multi\nline\"");
	cr_assert_eq(t.line, 2);

	init_scanner("\"no end");
	t = scan_token();
	cr_assert_eq(t.type, TOKEN_ERROR);
	cr_assert_eq(memcmp(t.start, "Unterminated string.", t.length), 0);
}

Test(scanner, tracks_line_numbers)
{
	init_scanner("var\nx\n=\n1;");
	Token t = scan_token();
	expect_token(t, TOKEN_VAR, "var");
	cr_assert_eq(t.line, 1);

	t = scan_token();
	expect_token(t, TOKEN_IDENTIFIER, "x");
	cr_assert_eq(t.line, 2);

	t = scan_token();
	expect_token(t, TOKEN_EQUAL, "=");
	cr_assert_eq(t.line, 3);

	t = scan_token();
	expect_token(t, TOKEN_NUMBER, "1");
	cr_assert_eq(t.line, 4);

	t = scan_token();
	expect_token(t, TOKEN_SEMICOLON, ";");
	cr_assert_eq(t.line, 4);
}

Test(scanner, simple_declaration_stream)
{
	init_scanner("var answer = 42;");
	expect_token(scan_token(), TOKEN_VAR, "var");
	expect_token(scan_token(), TOKEN_IDENTIFIER, "answer");
	expect_token(scan_token(), TOKEN_EQUAL, "=");
	expect_token(scan_token(), TOKEN_NUMBER, "42");
	expect_token(scan_token(), TOKEN_SEMICOLON, ";");
	expect_token(scan_token(), TOKEN_EOF, "");
}
