/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOENHANCEDPATHFORMULA_H
#define KOENHANCEDPATHFORMULA_H

#include <QString>
#include <QVariant>

class EnhancedPathShape;

class FormulaToken
{
public:
    /// token types
    enum Type
    {
      TypeUnknown = 0, ///< unknown type
      TypeNumber,      ///< 14, 3, 1977, 3.141592, 1e10, 5.9e-7
      TypeOperator,    ///< +, *, /, -
      TypeReference,   ///< function reference, modifier reference or named variable
      TypeFunction     ///< function name
    };

    /// operator types
    enum Operator {
        OperatorInvalid,   ///< invalid operator
        OperatorAdd,       ///< + addition
        OperatorSub,       ///< - subtraction
        OperatorMul,       ///< * multiplication
        OperatorDiv,       ///< / division
        OperatorLeftPar,   ///< (left parentheses
        OperatorRightPar,  ///<) right parentheses
        OperatorComma      ///< , comma
    };

    /// Constructs token with given type, text and position
    explicit FormulaToken(Type type = TypeUnknown, const QString & text = QString(), int position = -1);

    /// copy constructor
    FormulaToken(const FormulaToken &token);

    /// assignment operator
    FormulaToken& operator=(const FormulaToken &token);

    /// Returns the type of the token
    Type type() const { return m_type; }
    /// Returns the text representation of the token
    QString text() const { return m_text; }
    /// Returns the position of the token
    int position() const { return m_position; }

    /// Returns if the token is a number
    bool isNumber() const { return m_type == TypeNumber; }
    /// Returns if the token is a operator, OperatorInvalid if token is no operator
    bool isOperator() const { return m_type == TypeOperator; }
    /// Returns if token is a function
    bool isFunction() const { return m_type == TypeFunction; }
    /// Returns  if token is a reference
    bool isReference() const { return m_type == TypeReference; }

    /// Returns the token converted to qreal
    qreal asNumber() const;
    /// Returns the token as operator
    Operator asOperator() const;
private:
    Type m_type;    ///< the token type
    QString m_text; ///< the token text representation
    int m_position; ///< the tokens position
};

typedef QList<FormulaToken> TokenList;

class Opcode;

class EnhancedPathFormula
{
public:
    /// predefined functions
    enum Function {
        FunctionUnknown,
        // unary functions
        FunctionAbs,
        FunctionSqrt,
        FunctionSin,
        FunctionCos,
        FunctionTan,
        FunctionAtan,
        // binary functions
        FunctionAtan2,
        FunctionMin,
        FunctionMax,
        // ternary functions
        FunctionIf
    };

    /// The possible error code returned by error()
    enum Error {
        ErrorNone,    ///< no error
        ErrorValue,   ///< error when converting value
        ErrorParse,   ///< parsing error
        ErrorCompile, ///< compiling error
        ErrorName     ///< invalid function name value
    };

    /// Constructs a new formula from the specified string representation
    EnhancedPathFormula(const QString &text, EnhancedPathShape * parent);

    /// Destroys the formula
    ~EnhancedPathFormula();

    /**
     * Evaluates the formula using the given path as possible input.
     *
     * @param path the path to use as input
     * @return the evaluated result
     */
    qreal evaluate();

    /// Returns the last occurred error
    Error error() { return m_error; }

    /// returns string representaion of the formula
    QString toString() const;
private:
    /// Separates the given formula text into tokens.
    TokenList scan(const QString &formula) const;

    /// Compiles the formula tokens into byte code
    bool compile(const TokenList &tokens);

    /**
     * Evaluates a predefined function.
     *
     * @param function the identifier of the function to evaluate
     * @param arguments the functions arguments
     */
    qreal evaluateFunction(Function function, const QList<qreal> &arguments) const;

    /// Prints token list
    void debugTokens(const TokenList &tokens);
    /// Prints byte code
    void debugOpcodes();

    bool m_valid;    ///< flag that shows if function is valid, i.e the function was compiled successfully
    bool m_compiled; ///< flag that shows if function was compiled
    Error m_error;   ///< the last occurred error
    QString m_text; ///< the formula text representation
    QList<QVariant> m_constants; ///< constant values
    QList<Opcode> m_codes; ///< the compiled byte code
    EnhancedPathShape *m_parent;
};

#endif // KOENHANCEDPATHFORMULA_H
