/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004 Jarosław Staniek <staniek@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include <connection.h>
#include <tableschema.h>
#include "parser.h"
#include "parser_p.h"
#include "sqlparser.h"

/*moved to Driver
#include "tokens.cpp"
K_GLOBAL_STATIC_WITH_ARGS(StaticSetOfStrings, _reservedKeywords, (_tokens))
*/

//--------------------

using namespace KexiDB;

Parser::Parser(Connection *db)
        : d(new Private)
{
    d->db = db;
}

Parser::~Parser()
{
    delete d;
}

Parser::OPCode Parser::operation() const
{
    return (OPCode)d->operation;
}

QString
Parser::operationString() const
{
    switch ((OPCode)d->operation) {
    case OP_Error:
        return "Error";
    case OP_CreateTable:
        return "CreateTable";
    case OP_AlterTable:
        return "AlterTable";
    case OP_Select:
        return "Select";
    case OP_Insert:
        return "Insert";
    case OP_Update:
        return "Update";
    case OP_Delete:
        return "Delete";
    default: //OP_None
        return "None";
    }
}

TableSchema *Parser::table()
{
    TableSchema *t = d->table; d->table = 0; return t;
}

QuerySchema *Parser::query()
{
    QuerySchema *s = d->select; d->select = 0; return s;
}

Connection *Parser::db() const
{
    return d->db;
}

ParserError Parser::error() const
{
    return d->error;
}

QString Parser::statement() const
{
    return d->statement;
}

void Parser::setOperation(OPCode op)
{
    d->operation = op;
}

QuerySchema *Parser::select() const
{
    return d->select;
}

void Parser::setError(const ParserError &err)
{
    d->error = err;
}

void
Parser::createTable(const char *t)
{
    if (d->table)
        return;

    d->table = new KexiDB::TableSchema(t);
}

void
Parser::setQuerySchema(QuerySchema *query)
{
    if (d->select)
        delete d->select;

    d->select = query;
}

void Parser::init()
{
    if (d->initialized)
        return;
    // nothing to do
    d->initialized = true;
}

/*moved to Driver
bool Parser::isReservedKeyword(const QByteArray& str)
{
  return _reservedKeywords->contains(str.toUpper());
}*/

bool
Parser::parse(const QString &statement)
{
    init();
    clear();
    d->statement = statement;

    KexiDB::Parser *oldParser = parser;
    KexiDB::Field *oldField = field;
    bool res = parseData(this, statement.toUtf8());
    parser = oldParser;
    field = oldField;
    return res;
}

void
Parser::clear()
{
    d->clear();
}

//-------------------------------------

ParserError::ParserError()
        : m_at(-1)
{
// m_isNull = true;
}

ParserError::ParserError(const QString &type, const QString &error, const QString &hint, int at)
{
    m_type = type;
    m_error = error;
    m_hint = hint;
    m_at = at;
}

ParserError::~ParserError()
{
}

