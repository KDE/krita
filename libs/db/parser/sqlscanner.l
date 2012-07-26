/* This file is part of the KDE project
   Copyright (C) 2004 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004-2006 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

%{
#include <field.h>
#include <expression.h>

#include "sqlparser.h"
#include "sqltypes.h"
#include <iostream>
#include <kdebug.h>
#include <klocale.h>

#define YY_NO_UNPUT
#define ECOUNT current += yyleng; ctoken = yytext

extern void setError(const QString& errDesc);
extern void setError(const QString& errName, const QString& errDesc);

%}

/* *** Please reflect changes to this file in ../driver_p.cpp *** */

%option case-insensitive
%option noyywrap
%option never-interactive

whitespace	[ \t\n]
digit				[0-9]
/*identifier	[a-zA-Z_][a-zA-Z_0-9]* */
identifier	[a-zA-Z_0-9]+
/* quoted_identifier (\"[a-zA-Z_0-9]+\") */
query_parameter	\[[^\[\]]+\]

integer			{digit}+
decimal			(({digit}*\.{digit}+)|({digit}+\.{digit}*))
real				((({digit}*\.{digit}+)|({digit}+\.{digit}*)|({digit}+))([Ee][-+]?{digit}+))
/* todo: support for real numbers */


%%


"<>" {
	ECOUNT;
	return NOT_EQUAL;
}

"!=" {
	ECOUNT;
	return NOT_EQUAL2;
}

"==" {
	ECOUNT;
	return '=';
}

"<=" {
	ECOUNT;
	return LESS_OR_EQUAL;
}

">=" {
	ECOUNT;
	return GREATER_OR_EQUAL;
}

"IN" {
	ECOUNT;
	return SQL_IN;
}

{integer} {
//TODO: what about hex or octal values?
	//we're using QString:toLongLong() here because atoll() is not so portable:
	ECOUNT;
	bool ok;
	yylval.integerValue = QString(yytext).toLongLong( &ok );
	if (!ok) {
		setError(i18n("Invalid integer number"),i18n("This integer number may be too large."));
		return SCAN_ERROR;
	}
//	yylval.integerValue = atol(yytext);
	return INTEGER_CONST;
}

{decimal} {
	char *p = yytext;
	if (yytext[0]=='.') { /* no integer part */
		yylval.realValue.integer = 0;
	}
	else {
		yylval.realValue.integer = atoi(p);
		int i=0;
		while (p && i < yyleng && *p != '.') {
			i++;
			p++;
		}
		if (i==0 || !p || *p!='.') {
			yylval.realValue.fractional = 0;
			return REAL_CONST;
		}
	}
	/* fractional part */
	p++;
	yylval.realValue.fractional = atoi(p);
	return REAL_CONST;
}

("AND"|"&&") {
	ECOUNT;
	return AND;
}

"AS" {
	ECOUNT;
	return AS;
}

"CREATE" {
	ECOUNT;
	return CREATE;
}

"FROM" {
	ECOUNT;
	return FROM;
}

"INTEGER" {
	ECOUNT;
	return SQL_TYPE;
}

"JOIN" {
	ECOUNT;
	return JOIN;
}

"LEFT" {
	ECOUNT;
	return LEFT;
}

"LIKE" {
	ECOUNT;
	return LIKE;
}

"NOT"{whitespace}+"SIMILAR"{whitespace}+"TO" {
	ECOUNT;
	return NOT_SIMILAR_TO;
}

"SIMILAR"{whitespace}+"TO" {
	ECOUNT;
	return SIMILAR_TO;
}

"IS"{whitespace}+"NOT"{whitespace}+"NULL" {
	ECOUNT;
	return SQL_IS_NOT_NULL;
}

"IS"{whitespace}+"NULL" {
	ECOUNT;
	return SQL_IS_NULL;
}

"NOT" {
	ECOUNT;
	return NOT;
}

"IS" {
	ECOUNT;
	return SQL_IS;
}

"NULL" {
	ECOUNT;
	return SQL_NULL;
}

"ON" {
	ECOUNT;
	return SQL_ON;
}

"OR" {
	ECOUNT;
	return OR;
}

"||" { /* also means OR for numbers (mysql) */
	ECOUNT;
	return CONCATENATION;
}

"<<" {
	ECOUNT;
	return BITWISE_SHIFT_LEFT;
}

">>" {
	ECOUNT;
	return BITWISE_SHIFT_RIGHT;
}

"XOR" {
	ECOUNT;
	return XOR;
}

"RIGHT" {
	ECOUNT;
	return RIGHT;
}

"SELECT" {
	ECOUNT;
	return SELECT;
}

"TABLE" {
	ECOUNT;
	return TABLE;
}

"WHERE" {
	ECOUNT;
	return WHERE;
}

"ORDER" {
	ECOUNT;
	return ORDER;
}

"BY" {
	ECOUNT;
	return BY;
}

"ASC" {
	ECOUNT;
	return ASC;
}

"DESC" {
	ECOUNT;
	return DESC;
}

(['][^']*[']|["][^\"]*["]) {
	ECOUNT;
	yylval.stringValue = new QString(QString::fromUtf8(yytext+1, yyleng-2));
	return CHARACTER_STRING_LITERAL;

/* "ZZZ" sentinel for script */
}

{identifier} {
	KexiDBDbg << "yytext: '" << yytext << "' (" << yyleng << ")";
	ECOUNT;
	yylval.stringValue = new QString(QString::fromUtf8(yytext, yyleng));
	if (yytext[0]>='0' && yytext[0]<='9') {
		setError(i18n("Invalid identifier"),
		  i18n("Identifiers should start with a letter or '_' character"));
		return SCAN_ERROR;
	}
	return IDENTIFIER;
}

{query_parameter} {
	KexiDBDbg << "yytext: '" << yytext << "' (" << yyleng << ")";
	ECOUNT;
	yylval.stringValue = new QString(QString::fromUtf8(yytext+1, yyleng-2));
	return QUERY_PARAMETER;
}

{whitespace}+ {
	ECOUNT;
}

[\~\!\@\#\^\&\|\`\?,()\[\]\.;\:\+\-\*\/\%\^\<\>\=] {
	KexiDBDbg << "char: '" << yytext[0] << "'";
	ECOUNT;
	return yytext[0];
}

%%

void tokenize(const char *data)
{
	yy_switch_to_buffer(yy_scan_string(data));
	ctoken = "";
	current = 0;
}

