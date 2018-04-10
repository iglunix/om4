/* $OpenBSD: tokenizer.l,v 1.10 2017/06/17 01:55:16 bcallah Exp $ */
/*
 * Copyright (c) 2004 Marc Espie <espie@cvs.openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "parser.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern void m4_warnx(const char *, ...);
extern int mimic_gnu;
extern int32_t yylval;
static const char *yypos;

void
yy_scan_string(const char *s)
{
	yypos = s;
}

static int32_t
number(const char *yytext, size_t yylen)
{
	long l;

	errno = 0;
	l = strtol(yytext, NULL, 0);
	if (((l == LONG_MAX || l == LONG_MIN) && errno == ERANGE) ||
	    l > INT32_MAX || l < INT32_MIN)
		m4_warnx("numeric overflow in expr: %.*s", (int)yylen, yytext);
	return l;
}

static int32_t
parse_radix(const char *yytext, size_t yylen)
{
	long base;
	char *next;
	long l;
	int d;

	l = 0;
	base = strtol(yytext+2, &next, 0);
	if (base > 36 || next == NULL) {
		m4_warnx("error in number %.*s", (int)yylen, yytext);
	} else {
		next++;
		while (*next != 0) {
			if (*next >= '0' && *next <= '9')
				d = *next - '0';
			else if (*next >= 'a' && *next <= 'z')
				d = *next - 'a' + 10;
			else {
				assert(*next >= 'A' && *next <= 'Z');
				d = *next - 'A' + 10;
			}
			if (d >= base) {
				m4_warnx("error in number %.*s", (int)yylen, yytext);
				return 0;
			}
			l = base * l + d;
			next++;
		}
	}
	return l;
}

static int
isodigit(int c)
{
	return c >= '0' && c <= '7';
}

int yylex(void)
{
	const char *start;

next:
	start = yypos;
	switch (*yypos) {
	case ' ':
	case '\t':
	case '\n':
		++yypos;
		goto next;
	case '<':
		switch (yypos[1]) {
		case '=':
			yypos += 2;
			return LE;
		case '<':
			yypos += 2;
			return LSHIFT;
		}
		break;
	case '>':
		switch (yypos[1]) {
		case '=':
			yypos += 2;
			return GE;
		case '>':
			yypos += 2;
			return RSHIFT;
		}
		break;
	case '=':
		if (yypos[1] != '=')
			break;
		yypos += 2;
		return EQ;
	case '!':
		if (yypos[1] != '=')
			break;
		yypos += 2;
		return NE;
	case '&':
		if (yypos[1] != '&')
			break;
		yypos += 2;
		return LAND;
	case '|':
		if (yypos[1] != '|')
			break;
		yypos += 2;
		return LOR;
	case '*':
		if (!mimic_gnu || yypos[1] != '*')
			break;
		yypos += 2;
		return EXPONENT;
	case '0':
		switch (*++yypos) {
		case 'x':
		case 'X':
			if (!isxdigit(*++yypos))
				return ERROR;
			do ++yypos;
			while (isxdigit(*yypos));
			break;
		case 'r':
		case 'R':
			if (!mimic_gnu)
				break;
			if (!isdigit(*++yypos))
				return ERROR;
			do ++yypos;
			while (isdigit(*yypos));
			if (*yypos != ':')
				return ERROR;
			if (!isalnum(*++yypos))
				return ERROR;
			do ++yypos;
			while (isalnum(*yypos));
			yylval = parse_radix(start, yypos - start);
			return NUMBER;
		default:
			do ++yypos;
			while (isodigit(*yypos));
			break;
		}
		yylval = number(start, yypos - start);
		return NUMBER;
	case '\0':
		return '\0';
	}
	if (isdigit(*yypos)) {
		do ++yypos;
		while (isdigit(*yypos));
		yylval = number(start, yypos - start);
		return NUMBER;
	}

	return *yypos++;
}
