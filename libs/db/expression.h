/* This file is part of the KDE project
   Copyright (C) 2003-2007 Jarosław Staniek <staniek@kde.org>

   Design based on nexp.h : Parser module of Python-like language
   (C) 2001 Jarosław Staniek, MIMUW (www.mimuw.edu.pl)

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

#ifndef KEXIDB_EXPRESSION_H
#define KEXIDB_EXPRESSION_H

#include "field.h"
#include "queryschema.h"

#include <kdebug.h>
#include "global.h"
#include <QList>
#include <QByteArray>

namespace KexiDB
{

//! classes
#define KexiDBExpr_Unknown 0
#define KexiDBExpr_Unary 1
#define KexiDBExpr_Arithm 2
#define KexiDBExpr_Logical 3
#define KexiDBExpr_Relational 4
#define KexiDBExpr_SpecialBinary 5
#define KexiDBExpr_Const 6
#define KexiDBExpr_Variable 7
#define KexiDBExpr_Function 8
#define KexiDBExpr_Aggregation 9
#define KexiDBExpr_TableList 10
#define KexiDBExpr_QueryParameter 11

//! Custom tokens are not used in parser but used as extension in expression classes.
//#define KEXIDB_CUSTOM_TOKEN 0x1000

//! \return class name of class \a c
CALLIGRADB_EXPORT QString exprClassName(int c);

class ParseInfo;
class NArgExpr;
class UnaryExpr;
class BinaryExpr;
class ConstExpr;
class VariableExpr;
class FunctionExpr;
class QueryParameterExpr;
class QuerySchemaParameterValueListIterator;
//class QuerySchemaParameterList;

//! A base class for all expressions
class CALLIGRADB_EXPORT BaseExpr
{
public:
    typedef QList<BaseExpr*> List;
    typedef QList<BaseExpr*>::ConstIterator ListIterator;

    BaseExpr(int token);
    virtual ~BaseExpr();

    //! \return a deep copy of this object.
//! @todo a nonpointer will be returned here when we move to implicit data sharing
    virtual BaseExpr* copy() const = 0;

    int token() const {
        return m_token;
    }

    virtual Field::Type type();

    BaseExpr* parent() const {
        return m_par;
    }

    virtual void setParent(BaseExpr *p) {
        m_par = p;
    }

    virtual bool validate(ParseInfo& parseInfo);

    /*! \return string as a representation of this expression element by running recursive calls.
     \a param, if not 0, points to a list item containing value of a query parameter
     (used in QueryParameterExpr). */
    virtual QString toString(QuerySchemaParameterValueListIterator* params = 0) = 0;

    /*! Collects query parameters (messages and types) reculsively and saves them to params.
     The leaf nodes are objects of QueryParameterExpr class. */
    virtual void getQueryParameters(QuerySchemaParameterList& params) = 0;

    inline void debug() {
        KexiDBDbg << debugString();
    }

    virtual QString debugString();

    /*! \return single character if the token is < 256
     or token name, e.g. LESS_OR_EQUAL (for debugging). */
    inline QString tokenToDebugString() {
        return tokenToDebugString(m_token);
    }

    static QString tokenToDebugString(int token);

    /*! \return string for token, like "<=" or ">" */
    virtual QString tokenToString();

    int exprClass() const {
        return m_cl;
    }

    /*! Convenience type casts. */
    NArgExpr* toNArg();
    UnaryExpr* toUnary();
    BinaryExpr* toBinary();
    ConstExpr* toConst();
    VariableExpr* toVariable();
    FunctionExpr* toFunction();
    QueryParameterExpr* toQueryParameter();

protected:
    int m_cl; //!< class
    BaseExpr *m_par; //!< parent expression
    int m_token;
};

//! A base class N-argument operation
class CALLIGRADB_EXPORT NArgExpr : public BaseExpr
{
public:
    NArgExpr(int aClass, int token);
    NArgExpr(const NArgExpr& expr);
    virtual ~NArgExpr();
    //! \return a deep copy of this object.
    virtual NArgExpr* copy() const;
    void add(BaseExpr *expr);
    void prepend(BaseExpr *expr);
    BaseExpr *arg(int n);
    int args();
    virtual QString debugString();
    virtual QString toString(QuerySchemaParameterValueListIterator* params = 0);
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
    BaseExpr::List list;
};

//! An unary argument operation: + - NOT (or !) ~ "IS NULL" "IS NOT NULL"
class CALLIGRADB_EXPORT UnaryExpr : public BaseExpr
{
public:
    UnaryExpr(int token, BaseExpr *arg);
    UnaryExpr(const UnaryExpr& expr);
    virtual ~UnaryExpr();
    //! \return a deep copy of this object.
    virtual UnaryExpr* copy() const;
    virtual Field::Type type();
    virtual QString debugString();
    virtual QString toString(QuerySchemaParameterValueListIterator* params = 0);
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    BaseExpr *arg() const {
        return m_arg;
    }
    virtual bool validate(ParseInfo& parseInfo);

    BaseExpr *m_arg;
};

/*! A base class for binary operation
 - arithmetic operations: + - / * % << >> & | ||
 - relational operations: = (or ==) < > <= >= <> (or !=) LIKE IN 'SIMILAR TO' 'NOT SIMILAR TO'
 - logical operations: OR (or ||) AND (or &&) XOR
 - SpecialBinary "pseudo operators":
    * e.g. "f1 f2" : token == 0
    * e.g. "f1 AS f2" : token == AS
*/
class CALLIGRADB_EXPORT BinaryExpr : public BaseExpr
{
public:
    BinaryExpr(int aClass, BaseExpr *left_expr, int token, BaseExpr *right_expr);
    BinaryExpr(const BinaryExpr& expr);
    virtual ~BinaryExpr();
    //! \return a deep copy of this object.
    virtual BinaryExpr* copy() const;
    virtual Field::Type type();
    virtual QString debugString();
    virtual QString toString(QuerySchemaParameterValueListIterator* params = 0);
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    BaseExpr *left() const {
        return m_larg;
    }
    BaseExpr *right() const {
        return m_rarg;
    }
    virtual bool validate(ParseInfo& parseInfo);
    virtual QString tokenToString();

    BaseExpr *m_larg;
    BaseExpr *m_rarg;
};

/*! String, integer, float constants also includes NULL value.
 token can be: IDENTIFIER, SQL_NULL, CHARACTER_STRING_LITERAL,
 INTEGER_CONST, REAL_CONST */
class CALLIGRADB_EXPORT ConstExpr : public BaseExpr
{
public:
    ConstExpr(int token, const QVariant& val);
    ConstExpr(const ConstExpr& expr);
    virtual ~ConstExpr();
    //! \return a deep copy of this object.
    virtual ConstExpr* copy() const;
    virtual Field::Type type();
    virtual QString debugString();
    virtual QString toString(QuerySchemaParameterValueListIterator* params = 0);
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
    QVariant value;
};

//! Query parameter used to getting user input of constant values.
//! It contains a message that is displayed to the user.
class CALLIGRADB_EXPORT QueryParameterExpr : public ConstExpr
{
public:
    QueryParameterExpr(const QString& message);
    QueryParameterExpr(const QueryParameterExpr& expr);
    virtual ~QueryParameterExpr();
    //! \return a deep copy of this object.
    virtual QueryParameterExpr* copy() const;
    virtual Field::Type type();
    /*! Sets expected type of the parameter. The default is String.
     This method is called from parent's expression validate().
     This depends on the type of the related expression.
     For instance: query "SELECT * FROM cars WHERE name=[enter name]",
     "[enter name]" has parameter of the same type as "name" field.
     "=" binary expression's validate() will be called for the left side
     of the expression and then the right side will have type set to String.
    */
    void setType(Field::Type type);
    virtual QString debugString();
    virtual QString toString(QuerySchemaParameterValueListIterator* params = 0);
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);
protected:
    Field::Type m_type;
};

//! Variables like <i>fieldname</i> or <i>tablename</i>.<i>fieldname</i>
class CALLIGRADB_EXPORT VariableExpr : public BaseExpr
{
public:
    VariableExpr(const QString& _name);
    VariableExpr(const VariableExpr& expr);
    virtual ~VariableExpr();
    //! \return a deep copy of this object.
    virtual VariableExpr* copy() const;
    virtual Field::Type type();
    virtual QString debugString();
    virtual QString toString(QuerySchemaParameterValueListIterator* params = 0);
    virtual void getQueryParameters(QuerySchemaParameterList& params);

    /*! Validation. Sets field, tablePositionForField
     and tableForQueryAsterisk members.
     See addColumn() in parse.y to see how it's used on column adding. */
    virtual bool validate(ParseInfo& parseInfo);

    /*! Verbatim name as returned by scanner. */
    QString name;

    /* NULL by default. After successful validate() it will point to a field,
     if the variable is of a form "tablename.fieldname" or "fieldname",
     otherwise (eg. for asterisks) -still NULL.
     Only meaningful for column expressions within a query. */
    Field *field;

    /* -1 by default. After successful validate() it will contain a position of a table
     within query that needs to be bound to the field.
     This value can be either be -1 if no binding is needed.
     This value is used in the Parser to call
      QuerySchema::addField(Field* field, int bindToTable);
     Only meaningful for column expressions within a query. */
    int tablePositionForField;

    /*! NULL by default. After successful validate() it will point to a table
     that is referenced by asterisk, i.e. "*.tablename".
     This is set to NULL if this variable is not an asterisk of that form. */
    TableSchema *tableForQueryAsterisk;
};

//! - aggregation functions like SUM, COUNT, MAX, ...
//! - builtin functions like CURRENT_TIME()
//! - user defined functions
class CALLIGRADB_EXPORT FunctionExpr : public BaseExpr
{
public:
    FunctionExpr(const QString& _name, NArgExpr* args_ = 0);
    FunctionExpr(const FunctionExpr& expr);
    virtual ~FunctionExpr();
    //! \return a deep copy of this object.
    virtual FunctionExpr* copy() const;
    virtual Field::Type type();
    virtual QString debugString();
    virtual QString toString(QuerySchemaParameterValueListIterator* params = 0);
    virtual void getQueryParameters(QuerySchemaParameterList& params);
    virtual bool validate(ParseInfo& parseInfo);

    static QList<QByteArray> builtInAggregates();
    static bool isBuiltInAggregate(const QByteArray& fname);

    QString name;
    NArgExpr* args;
};

} //namespace KexiDB

#endif
