/* This file is part of the KDE project
   Copyright (C) 2004-2007 Jarosław Staniek <staniek@kde.org>

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

#include "parser_p.h"
#include "sqlparser.h"
#include "db/utils.h"

#include <QRegExp>
#include <QMutableListIterator>

#include <kdebug.h>
#include <klocale.h>

#include <assert.h>

using namespace KexiDB;

Parser *parser = 0;
Field *field = 0;
QList<Field*> fieldList;
int current = 0;
QByteArray ctoken;

extern int yylex_destroy(void);

//-------------------------------------

Parser::Private::Private()
        : initialized(false)
{
    clear();
    table = 0;
    select = 0;
    db = 0;
}

Parser::Private::~Private()
{
    delete select;
    delete table;
}

void Parser::Private::clear()
{
    operation = Parser::OP_None;
    error = ParserError();
}

//-------------------------------------

ParseInfo::ParseInfo(KexiDB::QuerySchema *query)
        : querySchema(query)
{
//Qt 4 repeatedTablesAndAliases.setAutoDelete(true);
}

ParseInfo::~ParseInfo()
{
}

//-------------------------------------

extern int yyparse();
extern void tokenize(const char *data);

void yyerror(const char *str)
{
    KexiDBDbg << "error: " << str;
    KexiDBDbg << "at character " << current << " near tooken " << ctoken;
    parser->setOperation(Parser::OP_Error);

    const bool otherError = (qstrnicmp(str, "other error", 11) == 0);
    const bool syntaxError = qstrnicmp(str, "syntax error", 12) == 0;
    if ((parser->error().type().isEmpty()
            && (str == 0 || strlen(str) == 0 || syntaxError))
            || otherError)
    {
        KexiDBDbg << parser->statement();
        QString ptrline(current, QLatin1Char(' '));

        ptrline += "^";
        KexiDBDbg << ptrline;

#if 0
        //lexer may add error messages
        QString lexerErr = parser->error().error();

        QString errtypestr(str);
        if (lexerErr.isEmpty()) {
            if (errtypestr.startsWith("parse error, unexpected ")) {
                //something like "parse error, unexpected IDENTIFIER, expecting ',' or ')'"
                QString e = errtypestr.mid(24);
                KexiDBDbg << e;
                QString token = "IDENTIFIER";
                if (e.startsWith(token)) {
                    QRegExp re("'.'");
                    int pos = 0;
                    pos = re.search(e, pos);
                    QStringList captured = re.capturedTexts();
                    if (captured.count() >= 2) {
//      KexiDBDbg << "**" << captured.at(1);
//      KexiDBDbg << "**" << captured.at(2);
                    }
                }



//    IDENTIFIER, expecting '")) {
                e = errtypestr.mid(47);
                KexiDBDbg << e;
//    ,' or ')'
//  lexerErr futureI18n("identifier was expected");

            } else
                if (errtypestr.startsWith("parse error, expecting `IDENTIFIER'"))
                    lexerErr = i18n("identifier was expected");
        }
#endif

        //! @todo exact invalid expression can be selected in the editor, based on ParseInfo data

        if (!otherError) {
            const bool isKexiSQLKeyword = KexiDB::isKexiSQLKeyword(ctoken);
            if (isKexiSQLKeyword || syntaxError) {
                if (isKexiSQLKeyword) {
                    parser->setError(ParserError(i18n("Syntax Error"),
                                                 i18n("\"%1\" is a reserved keyword.", QString(ctoken)),
                                                 ctoken, current));
                } else {
                    parser->setError(ParserError(i18n("Syntax Error"),
                                                 i18n("Syntax error."),
                                                 ctoken, current));
                }
            } else {
                parser->setError(ParserError(i18n("Error"),
                                             i18n("Error near \"%1\".", QString(ctoken)),
                                             ctoken, current));
            }
        }
    }
}

void setError(const QString& errName, const QString& errDesc)
{
    parser->setError(ParserError(errName, errDesc, ctoken, current));
    yyerror(errName.toLatin1());
}

void setError(const QString& errDesc)
{
    setError("other error", errDesc);
}

/* this is better than assert() */
#define IMPL_ERROR(errmsg) setError("Implementation error", errmsg)

bool parseData(Parser *p, const char *data)
{
    /* todo: make this REENTRANT */
    parser = p;
    parser->clear();
    field = 0;
    fieldList.clear();

    if (!data) {
        ParserError err(i18n("Error"), i18n("No query statement specified."), ctoken, current);
        parser->setError(err);
        yyerror("");
        parser = 0;
        return false;
    }

    tokenize(data);
    if (!parser->error().type().isEmpty()) {
        parser = 0;
        return false;
    }
    yyparse();

    bool ok = true;
    if (parser->operation() == Parser::OP_Select) {
        KexiDBDbg << "parseData(): ok";
        //   KexiDBDbg << "parseData(): " << tableDict.count() << " loaded tables";
        /*   TableSchema *ts;
              for(QDictIterator<TableSchema> it(tableDict); TableSchema *s = tableList.first(); s; s = tableList.next())
              {
                KexiDBDbg << " " << s->name();
              }*/
    } else {
        ok = false;
    }
    yylex_destroy();
    parser = 0;
    return ok;
}


/* Adds \a column to \a querySchema. \a column can be in a form of
 table.field, tableAlias.field or field
*/
bool addColumn(ParseInfo& parseInfo, BaseExpr* columnExpr)
{
    if (!columnExpr->validate(parseInfo)) {
        setError(parseInfo.errMsg, parseInfo.errDescr);
        return false;
    }

    VariableExpr *v_e = columnExpr->toVariable();
    if (columnExpr->exprClass() == KexiDBExpr_Variable && v_e) {
        //it's a variable:
        if (v_e->name == "*") {//all tables asterisk
            if (parseInfo.querySchema->tables()->isEmpty()) {
                setError(i18n("\"*\" could not be used if no tables are specified."));
                return false;
            }
            parseInfo.querySchema->addAsterisk(new QueryAsterisk(parseInfo.querySchema));
        } else if (v_e->tableForQueryAsterisk) {//one-table asterisk
            parseInfo.querySchema->addAsterisk(
                new QueryAsterisk(parseInfo.querySchema, v_e->tableForQueryAsterisk));
        } else if (v_e->field) {//"table.field" or "field" (bound to a table or not)
            parseInfo.querySchema->addField(v_e->field, v_e->tablePositionForField);
        } else {
            IMPL_ERROR("addColumn(): unknown case!");
            return false;
        }
        return true;
    }

    //it's complex expression
    parseInfo.querySchema->addExpression(columnExpr);
    return true;
}

//! clean up no longer needed temporary objects
#define CLEANUP \
    delete colViews; \
    delete tablesList; \
    delete options

QuerySchema* buildSelectQuery(
    QuerySchema* querySchema, NArgExpr* colViews, NArgExpr* tablesList,
    SelectOptionsInternal* options)
{
    ParseInfo parseInfo(querySchema);

    //-------tables list
// assert( tablesList ); //&& tablesList->exprClass() == KexiDBExpr_TableList );

    uint columnNum = 0;
    /*TODO: use this later if there are columns that use database fields,
            e.g. "SELECT 1 from table1 t, table2 t") is ok however. */
    //used to collect information about first repeated table name or alias:
// QDict<char> tableNamesAndTableAliases(997, false);
// QString repeatedTableNameOrTableAlias;
    if (tablesList) {
        for (int i = 0; i < tablesList->args(); ++i, ++columnNum) {
            BaseExpr *e = tablesList->arg(i);
            VariableExpr* t_e = 0;
            QString aliasString;
            if (e->exprClass() == KexiDBExpr_SpecialBinary) {
                BinaryExpr* t_with_alias = e->toBinary();
                assert(t_with_alias);
                assert(t_with_alias->left()->exprClass() == KexiDBExpr_Variable);
                assert(t_with_alias->right()->exprClass() == KexiDBExpr_Variable
                       && (t_with_alias->token() == AS || t_with_alias->token() == 0));
                t_e = t_with_alias->left()->toVariable();
                aliasString = t_with_alias->right()->toVariable()->name.toLatin1();
            } else {
                t_e = e->toVariable();
            }
            assert(t_e);
            QString tname = t_e->name;
            TableSchema *s = parser->db()->tableSchema(tname);
            if (!s) {
                setError(
                    i18n("Table \"%1\" does not exist.", tname));
                CLEANUP;
                return 0;
            }
            QString tableOrAliasName = KexiDB::iifNotEmpty(aliasString, tname);
            if (!aliasString.isEmpty()) {
//    KexiDBDbg << "- add alias for table: " << aliasString;
            }
            // 1. collect information about first repeated table name or alias
            //    (potential ambiguity)
            QList<int> list(parseInfo.repeatedTablesAndAliases.value(tableOrAliasName));
            list.append(i);
            parseInfo.repeatedTablesAndAliases.insert(tableOrAliasName, list);
            /*  if (repeatedTableNameOrTableAlias.isEmpty()) {
                  if (tableNamesAndTableAliases[tname])
                    repeatedTableNameOrTableAlias=tname;
                  else
                    tableNamesAndTableAliases.insert(tname, (const char*)1);
                }
                if (!aliasString.isEmpty()) {
                  KexiDBDbg << "- add alias for table: " << aliasString;
            //   querySchema->setTableAlias(columnNum, aliasString);
                  //2. collect information about first repeated table name or alias
                  //   (potential ambiguity)
                  if (repeatedTableNameOrTableAlias.isEmpty()) {
                    if (tableNamesAndTableAliases[aliasString])
                      repeatedTableNameOrTableAlias=aliasString;
                    else
                      tableNamesAndTableAliases.insert(aliasString, (const char*)1);
                  }
                }*/
//   KexiDBDbg << "addTable: " << tname;
            querySchema->addTable(s, aliasString.toLatin1());
        }
    }

    /* set parent table if there's only one */
// if (parser->select()->tables()->count()==1)
    if (querySchema->tables()->count() == 1)
        querySchema->setMasterTable(querySchema->tables()->first());

    //-------add fields
    if (colViews) {
        columnNum = 0;
        for (QMutableListIterator<BaseExpr*> it(colViews->list); it.hasNext(); ++columnNum) {
            BaseExpr *e = it.next();
//Qt4   bool moveNext = true; //used to avoid ++it when an item is taken from the list
            BaseExpr *columnExpr = e;
            VariableExpr* aliasVariable = 0;
            if (e->exprClass() == KexiDBExpr_SpecialBinary && e->toBinary()
                    && (e->token() == AS || e->token() == 0)) {
                //KexiDBExpr_SpecialBinary: with alias
                columnExpr = e->toBinary()->left();
                //   isFieldWithAlias = true;
                aliasVariable = e->toBinary()->right()->toVariable();
                if (!aliasVariable) {
                    setError(i18n("Invalid alias definition for column \"%1\".",
                                  columnExpr->toString(0))); //ok?
                    break;
                }
            }

            const int c = columnExpr->exprClass();
            const bool isExpressionField =
                c == KexiDBExpr_Const
                || c == KexiDBExpr_Unary
                || c == KexiDBExpr_Arithm
                || c == KexiDBExpr_Logical
                || c == KexiDBExpr_Relational
                || c == KexiDBExpr_Function
                || c == KexiDBExpr_Aggregation;

            if (c == KexiDBExpr_Variable) {
                //just a variable, do nothing, addColumn() will handle this
            } else if (isExpressionField) {
                //expression object will be reused, take, will be owned, do not destroy
//  KexiDBDbg << colViews->list.count() << " " << it.current()->debugString();
                it.remove();
//Qt4    moveNext = false;
            } else if (aliasVariable) {
                //take first (left) argument of the special binary expr, will be owned, do not destroy
                e->toBinary()->m_larg = 0;
            } else {
                setError(i18n("Invalid \"%1\" column definition.", e->toString(0))); //ok?
                break;
            }

            if (!addColumn(parseInfo, columnExpr)) {
                break;
            }

            if (aliasVariable) {
//    KexiDBDbg << "ALIAS \"" << aliasVariable->name << "\" set for column "
//     << columnNum;
                querySchema->setColumnAlias(columnNum, aliasVariable->name.toLatin1());
            }
            /*  if (e->exprClass() == KexiDBExpr_SpecialBinary && dynamic_cast<BinaryExpr*>(e)
                  && (e->type()==AS || e->type()==0))
                {
                  //also add alias
                  VariableExpr* aliasVariable =
                    dynamic_cast<VariableExpr*>(dynamic_cast<BinaryExpr*>(e)->right());
                  if (!aliasVariable) {
                    setError(futureI18n("Invalid column alias definition")); //ok?
                    return 0;
                  }
                  kDebug() << "ALIAS \"" << aliasVariable->name << "\" set for column "
                    << columnNum;
                  querySchema->setColumnAlias(columnNum, aliasVariable->name.toLatin1());
                }*/

//Qt4   if (moveNext) {
//Qt4    colViews->list.next();
//Qt4   }
        } // for
        if (!parser->error().error().isEmpty()) { // we could not return earlier (inside the loop)
            // because we want run CLEANUP what could crash QMutableListIterator.
            CLEANUP;
            return 0;
        }
    }
    //----- SELECT options
    if (options) {
        //----- WHERE expr.
        if (options->whereExpr) {
            if (!options->whereExpr->validate(parseInfo)) {
                setError(parseInfo.errMsg, parseInfo.errDescr);
                CLEANUP;
                return 0;
            }
            querySchema->setWhereExpression(options->whereExpr);
        }
        //----- ORDER BY
        if (options->orderByColumns) {
            OrderByColumnList &orderByColumnList = querySchema->orderByColumnList();
            uint count = options->orderByColumns->count();
            OrderByColumnInternal::ListConstIterator it(options->orderByColumns->constEnd());
            --it;
            for (;count > 0; --it, --count)
                /*opposite direction due to parser specifics*/
            {
                //first, try to find a column name or alias (outside of asterisks)
                QueryColumnInfo *columnInfo = querySchema->columnInfo((*it).aliasOrName, false/*outside of asterisks*/);
                if (columnInfo) {
                    orderByColumnList.appendColumn(*columnInfo, (*it).ascending);
                } else {
                    //failed, try to find a field name within all the tables
                    if ((*it).columnNumber != -1) {
                        if (!orderByColumnList.appendColumn(*querySchema,
                                                            (*it).ascending, (*it).columnNumber - 1)) {
                            setError(i18n("Could not define sorting - no column at position %1.",
                                          (*it).columnNumber));
                            CLEANUP;
                            return 0;
                        }
                    } else {
                        Field * f = querySchema->findTableField((*it).aliasOrName);
                        if (!f) {
                            setError(i18n("Could not define sorting - "
                                          "column name or alias \"%1\" does not exist.", (*it).aliasOrName));
                            CLEANUP;
                            return 0;
                        }
                        orderByColumnList.appendField(*f, (*it).ascending);
                    }
                }
            }
        }
    }

// KexiDBDbg << "Select ColViews=" << (colViews ? colViews->debugString() : QString())
//  << " Tables=" << (tablesList ? tablesList->debugString() : QString()s);

    CLEANUP;
    return querySchema;
}

#undef CLEANUP

