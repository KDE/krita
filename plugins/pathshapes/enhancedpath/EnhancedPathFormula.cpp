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

#include "EnhancedPathFormula.h"
#include "EnhancedPathShape.h"
#include <QStack>
#include <math.h>

#include <kdebug.h>

/*
    The formula parsing, compiling and evaluating is based on
    kspreads formula engine written by Ariya Hidayat.
    There is a DESIGN.html file in the kspreads directory which
    explains how the engine is working.
    The engine was stripped down a little to only support the
    operations needed for the odf enhanced path formula spec.
*/

// helper function: return operator of given token text
FormulaToken::Operator matchOperator(const QString &text);
// helper function: return true for valid identifier character
bool isIdentifier(QChar ch);
// helper function: give operator precedence
// e.g. '+' is 1 while '*' is 3
int opPrecedence(FormulaToken::Operator op);
// helper function: return function of given token text
EnhancedPathFormula::Function matchFunction(const QString & text);
// helper function: return function name from function identifier
QString matchFunction(EnhancedPathFormula::Function function);

class FormulaToken;

class FormulaTokenStack : public QVector<FormulaToken>
{
public:
    FormulaTokenStack()
        : QVector<FormulaToken>(), topIndex(0)
    {
        ensureSpace();
    }

    bool isEmpty() const { return topIndex == 0; }
    unsigned itemCount() const { return topIndex; }
    void push(const FormulaToken& token)
    {
        ensureSpace();
        insert(topIndex++, token);
    }
    FormulaToken pop() { return (topIndex > 0) ? FormulaToken(at(--topIndex)) : FormulaToken(); }
    const FormulaToken& top() { return top(0); }
    const FormulaToken& top(unsigned index)
    {
        static FormulaToken null;
        if (topIndex > index)
            return at(topIndex-index-1);
        return null;
    }
private:
    void ensureSpace()
    {
        while((int) topIndex >= size())
            resize(size() + 10);
    }
    unsigned topIndex;
};

class Opcode
{
public:

  enum { Nop = 0, Load, Ref, Function, Add, Sub, Neg, Mul, Div };

  unsigned type;
  unsigned index;

  Opcode(): type(Nop), index(0) {}
  Opcode(unsigned t): type(t), index(0) {}
  Opcode(unsigned t, unsigned i): type(t), index(i) {}
};


EnhancedPathFormula::EnhancedPathFormula(const QString &text, EnhancedPathShape *parent)
    : m_valid(false), m_compiled(false), m_error(ErrorNone), m_text(text), m_parent(parent)
{
    Q_ASSERT(m_parent);
}

EnhancedPathFormula::~EnhancedPathFormula()
{
}

qreal EnhancedPathFormula::evaluate()
{
    // shortcut
    if (m_error != ErrorNone)
        return 0.0;

    // lazy evaluation
    if (!m_compiled) {
        TokenList tokens = scan(m_text);
        if (m_error != ErrorNone)
            debugTokens(tokens);
        if (!compile(tokens)) {
            debugOpcodes();
            m_error = ErrorCompile;
            return false;
        }
        m_compiled = true;
    }

    QStack<QVariant> stack;
    // stack.reserve(3) here so that the stack is not resized all the time
    // this reduces the number of a/de/re-llocations for documents with 
    // a lot of enhanced path shapes quite a lot.
    stack.reserve(3);
    int index = 0;

    if (!m_valid) {
        m_error = ErrorParse;
        return 0.0;
    }

    for (int pc = 0; pc < m_codes.count(); pc++) {
        QVariant ret;   // for the function caller
        Opcode& opcode = m_codes[pc];
        index = opcode.index;
        switch(opcode.type) {
        // no operation
        case Opcode::Nop:
            break;

        // load a constant, push to stack
        case Opcode::Load:
            stack.push(m_constants[index]);
        break;

        // unary operation
        case Opcode::Neg: {
            bool success = false;
            qreal value = stack.pop().toDouble(&success);
            if (success) // do nothing if we got an error
                value *= -1.0;
            stack.push(QVariant(value));
            break;
        }

        // binary operation: take two values from stack, do the operation,
        // push the result to stack
        case Opcode::Add: {
            qreal val2 = stack.pop().toDouble();
            qreal val1 = stack.pop().toDouble();
            stack.push(QVariant(val1 + val2));
            break;
        }

        case Opcode::Sub: {
            qreal val2 = stack.pop().toDouble();
            qreal val1 = stack.pop().toDouble();
            stack.push(QVariant(val1 - val2));
            break;
        }

        case Opcode::Mul: {
            qreal val2 = stack.pop().toDouble();
            qreal val1 = stack.pop().toDouble();
            stack.push(QVariant(val1 * val2));
            break;
        }

        case Opcode::Div: {
            qreal val2 = stack.pop().toDouble();
            qreal val1 = stack.pop().toDouble();
            stack.push(QVariant(val1 / val2));
            break;
        }

        case Opcode::Ref: {
            QString reference = m_constants[index].toString();
            // push function name if it is a function, else push evaluated reference
            Function function = matchFunction(reference);
            if (FunctionUnknown == function)
                stack.push(QVariant(m_parent->evaluateReference(reference)));
            else
                stack.push(function);
            break;
        }

        // calling function
        case Opcode::Function: {
            // sanity check, this should not happen unless opcode is wrong
            // (i.e. there's a bug in the compile() function)
            if (stack.count() < index) {
                kWarning() << "not enough arguments for function " << m_text;
                m_error = ErrorValue; // not enough arguments
                return 0.0;
            }

            /// prepare function arguments
            QList<qreal> args;
            for (; index; index--) {
                qreal value = stack.pop().toDouble();
                args.push_front(value);
            }

            // function identifier as int value
            int function = stack.pop().toInt();
            stack.push(QVariant(evaluateFunction((Function)function, args)));
            break;
        }

        default:
            break;
        }
    }

    // more than one value in stack ? unsuccessful execution...
    if (stack.count() != 1) {
        m_error = ErrorValue;
        return 0.0;
    }

    return stack.pop().toDouble();
}

qreal EnhancedPathFormula::evaluateFunction(Function function, const QList<qreal> &arguments) const
{
    switch(function) {
    case EnhancedPathFormula::FunctionAbs:
        return fabs(arguments[0]);
        break;
    case EnhancedPathFormula::FunctionSqrt:
        return sqrt(arguments[0]);
        break;
    case EnhancedPathFormula::FunctionSin:
        return sin(arguments[0]);
        break;
    case EnhancedPathFormula::FunctionCos:
        return cos(arguments[0]);
        break;
    case EnhancedPathFormula::FunctionTan:
        return tan(arguments[0]);
        break;
    case EnhancedPathFormula::FunctionAtan:
        return atan(arguments[0]);
        break;
    case EnhancedPathFormula::FunctionAtan2:
        // TODO atan2 with one argument as in odf spec ???
        return atan2(arguments[0], arguments[1]);
        break;
    case EnhancedPathFormula::FunctionMin:
        return qMin(arguments[0], arguments[1]);
        break;
    case EnhancedPathFormula::FunctionMax:
        return qMax(arguments[0], arguments[1]);
        break;
    case EnhancedPathFormula::FunctionIf:
        if (arguments[0] > 0.0)
            return arguments[1];
        else
            return arguments[2];
        break;
    default:
        return 0.0;
    }

    return 0.0;
}

TokenList EnhancedPathFormula::scan(const QString &formula) const
{
  // parsing state
    enum {
        Start,
        Finish,
        Bad,
        InNumber,
        InDecimal,
        InExpIndicator,
        InExponent,
        InString,
        InIdentifier
    } state;

    TokenList tokens;

    int i = 0;
    state = Start;
    int tokenStart = 0;
    QString tokenText;
    QString expr = formula + QChar();

    // main loop
    while((state != Bad) && (state != Finish) && (i < expr.length())) {
        QChar ch = expr[i];

        switch(state) {
        case Start:
            tokenStart = i;

            // skip any whitespaces
            if (ch.isSpace()) {
                i++;
            } else if (ch.isDigit()) { // check for number
                state = InNumber;
            }
            // beginning with alphanumeric ?
            // could be identifier, function, function reference, modifier reference
            else if (isIdentifier(ch)) {
                state = InIdentifier;
            } else if (ch == '.') { // decimal dot ?
                tokenText.append(expr[i++]);
                state = InDecimal;
            } else if (ch == QChar::Null) { // terminator character
                state = Finish;
            } else { // look for operator match
                QString opString(ch);
                int op = matchOperator(opString);

                // any matched operator ?
                if (op != FormulaToken::OperatorInvalid) {
                    i++;
                    tokens.append(FormulaToken(FormulaToken::TypeOperator, opString, tokenStart));
                } else {
                    state = Bad;
                }
            }
            break;
        case InIdentifier:
            // consume as long as alpha, dollar sign, question mark, or digit
            if (isIdentifier(ch) || ch.isDigit()) {
                tokenText.append(expr[i++]);
            } else if (ch == '(') { // a '(' ? then this must be a function identifier
                tokens.append(FormulaToken(FormulaToken::TypeFunction, tokenText, tokenStart));
                tokenStart = i;
                tokenText = "";
                state = Start;
            } else { // we're done with identifier
                tokens.append(FormulaToken(FormulaToken::TypeReference, tokenText, tokenStart));
                tokenStart = i;
                tokenText = "";
                state = Start;
            }
            break;
        case InNumber:
            // consume as long as it's digit
            if (ch.isDigit()) {
                tokenText.append(expr[i++]);
            } else if (ch == '.') { // decimal dot ?
                tokenText.append('.');
                i++;
                state = InDecimal;
            } else if (ch.toUpper() == 'E') { // exponent ?
                tokenText.append('E');
                i++;
                state = InExpIndicator;
            } else { // we're done with integer number
                tokens.append(FormulaToken(FormulaToken::TypeNumber, tokenText, tokenStart));
                tokenText = "";
                state = Start;
            };
            break;
        case InDecimal:
            // consume as long as it's digit
            if (ch.isDigit()) {
                tokenText.append(expr[i++]);
            } else if (ch.toUpper() == 'E') { // exponent ?
                tokenText.append('E');
                i++;
                state = InExpIndicator;
            } else { // we're done with floating-point number
                tokens.append(FormulaToken(FormulaToken::TypeNumber, tokenText, tokenStart));
                tokenText = "";
                state = Start;
            };
            break;
        case InExpIndicator:
            // possible + or - right after E, e.g 1.23E+12 or 4.67E-8
            if ((ch == '+') || (ch == '-')) {
                tokenText.append(expr[i++]);
            } else if (ch.isDigit()) { // consume as long as it's digit
                state = InExponent;
            } else { // invalid thing here
                state = Bad;
            }
            break;
        case InExponent:
            // consume as long as it's digit
            if (ch.isDigit()) {
                tokenText.append(expr[i++]);
            } else { // we're done with floating-point number
                tokens.append(FormulaToken(FormulaToken::TypeNumber, tokenText, tokenStart));
                tokenText = "";
                state = Start;
            }
            break;
        case Bad:
        default:
            break;
        }
    }
    return tokens;
}

bool EnhancedPathFormula::compile(const TokenList &tokens)
{
    // sanity check
    if (tokens.count() == 0)
        return false;

    FormulaTokenStack syntaxStack;
    QStack<int> argStack;
    unsigned argCount = 1;

    for (int i = 0; i <= tokens.count(); i++) {
        // helper token: InvalidOp is end-of-formula
        FormulaToken token =  (i < tokens.count()) ? tokens[i] : FormulaToken(FormulaToken::TypeOperator);
        FormulaToken::Type tokenType = token.type();

        // unknown token is invalid
        if (tokenType == FormulaToken::TypeUnknown)
            break;

        // for constants, push immediately to stack
        // generate code to load from a constant
        if (tokenType == FormulaToken::TypeNumber) {
            syntaxStack.push(token);
            m_constants.append(QVariant(token.asNumber()));
            m_codes.append(Opcode(Opcode::Load, m_constants.count()-1));
        }
        // for identifier, push immediately to stack
        // generate code to load from reference
        if (tokenType == FormulaToken::TypeFunction || tokenType == FormulaToken::TypeReference) {
            syntaxStack.push(token);
            m_constants.append(QVariant(token.text()));
            m_codes.append(Opcode(Opcode::Ref, m_constants.count()-1));
        }
        // are we entering a function ?
        // if token is operator, and stack already has: id (arg
        if (tokenType == FormulaToken::TypeOperator && syntaxStack.itemCount() >= 3) {
            FormulaToken arg = syntaxStack.top();
            FormulaToken par = syntaxStack.top(1);
            FormulaToken id = syntaxStack.top(2);
            if (!arg.isOperator() &&
                 par.asOperator() == FormulaToken::OperatorLeftPar &&
                 id.isFunction()) {
                argStack.push(argCount);
                argCount = 1;
            }
        }
        // for any other operator, try to apply all parsing rules
        if (tokenType == FormulaToken::TypeOperator) {
            // repeat until no more rule applies
            for (; ;) {
                bool ruleFound = false;

                // rule for function arguments, if token is , or)
                // id (arg1 , arg2 -> id (arg
                if (!ruleFound)
                if (syntaxStack.itemCount() >= 5)
                if ((token.asOperator() == FormulaToken::OperatorRightPar) ||
                    (token.asOperator() == FormulaToken::OperatorComma)) {
                    FormulaToken arg2 = syntaxStack.top();
                    FormulaToken sep = syntaxStack.top(1);
                    FormulaToken arg1 = syntaxStack.top(2);
                    FormulaToken par = syntaxStack.top(3);
                    FormulaToken id = syntaxStack.top(4);
                    if (!arg2.isOperator())
                      if (sep.asOperator() == FormulaToken::OperatorComma)
                        if (!arg1.isOperator())
                          if (par.asOperator() == FormulaToken::OperatorLeftPar)
                            if (id.isFunction()) {
                        ruleFound = true;
                        syntaxStack.pop();
                        syntaxStack.pop();
                        argCount++;
                    }
                }
                // rule for function last argument:
                //  id (arg) -> arg
                if (!ruleFound)
                  if (syntaxStack.itemCount() >= 4) {
                    FormulaToken par2 = syntaxStack.top();
                    FormulaToken arg = syntaxStack.top(1);
                    FormulaToken par1 = syntaxStack.top(2);
                    FormulaToken id = syntaxStack.top(3);
                    if (par2.asOperator() == FormulaToken::OperatorRightPar)
                      if (!arg.isOperator())
                        if (par1.asOperator() == FormulaToken::OperatorLeftPar)
                          if (id.isFunction()) {
                        ruleFound = true;
                        syntaxStack.pop();
                        syntaxStack.pop();
                        syntaxStack.pop();
                        syntaxStack.pop();
                        syntaxStack.push(arg);
                        m_codes.append(Opcode(Opcode::Function, argCount));
                        argCount = argStack.empty() ? 0 : argStack.pop();
                    }
                }

                // rule for parenthesis:  (Y) -> Y
                if (!ruleFound)
                  if (syntaxStack.itemCount() >= 3) {
                    FormulaToken right = syntaxStack.top();
                    FormulaToken y = syntaxStack.top(1);
                    FormulaToken left = syntaxStack.top(2);
                    if (right.isOperator())
                      if (!y.isOperator())
                        if (left.isOperator())
                          if (right.asOperator() == FormulaToken::OperatorRightPar)
                            if (left.asOperator() == FormulaToken::OperatorLeftPar) {
                        ruleFound = true;
                        syntaxStack.pop();
                        syntaxStack.pop();
                        syntaxStack.pop();
                        syntaxStack.push(y);
                    }
                }

                // rule for binary operator:  A (op) B -> A
                // conditions: precedence of op >= precedence of token
                // action: push (op) to result
                // e.g. "A * B" becomes 'A' if token is operator '+'
                if (!ruleFound)
                  if (syntaxStack.itemCount() >= 3) {
                    FormulaToken b = syntaxStack.top();
                    FormulaToken op = syntaxStack.top(1);
                    FormulaToken a = syntaxStack.top(2);
                    if (!a.isOperator())
                      if (!b.isOperator())
                        if (op.isOperator())
                          if (token.asOperator() != FormulaToken::OperatorLeftPar)
                            if (opPrecedence(op.asOperator()) >= opPrecedence(token.asOperator())) {
                        ruleFound = true;
                        syntaxStack.pop();
                        syntaxStack.pop();
                        syntaxStack.pop();
                        syntaxStack.push(b);
                        switch(op.asOperator()) {
                        // simple binary operations
                        case FormulaToken::OperatorAdd: m_codes.append(Opcode::Add); break;
                        case FormulaToken::OperatorSub: m_codes.append(Opcode::Sub); break;
                        case FormulaToken::OperatorMul: m_codes.append(Opcode::Mul); break;
                        case FormulaToken::OperatorDiv: m_codes.append(Opcode::Div); break;
                        default: break;
                        }
                    }
                }

                // rule for unary operator:  (op1) (op2) X -> (op1) X
                // conditions: op2 is unary, token is not '('
                // action: push (op2) to result
                // e.g.  "* - 2" becomes '*'
                if (!ruleFound)
                  if (token.asOperator() != FormulaToken::OperatorLeftPar)
                  if (syntaxStack.itemCount() >= 3) {
                    FormulaToken x = syntaxStack.top();
                    FormulaToken op2 = syntaxStack.top(1);
                    FormulaToken op1 = syntaxStack.top(2);
                    if (!x.isOperator())
                      if (op1.isOperator())
                        if (op2.isOperator())
                          if ((op2.asOperator() == FormulaToken::OperatorAdd)
                                  || (op2.asOperator() == FormulaToken::OperatorSub)) {
                        ruleFound = true;
                        syntaxStack.pop();
                        syntaxStack.pop();
                        syntaxStack.push(x);
                        if (op2.asOperator() == FormulaToken::OperatorSub)
                            m_codes.append(Opcode(Opcode::Neg));
                    }
                }

                // auxiliary rule for unary operator:  (op) X -> X
                // conditions: op is unary, op is first in syntax stack, token is not '('
                // action: push (op) to result
                if (!ruleFound)
                  if (token.asOperator() != FormulaToken::OperatorLeftPar)
                    if (syntaxStack.itemCount() == 2) {
                    FormulaToken x = syntaxStack.top();
                    FormulaToken op = syntaxStack.top(1);
                    if (!x.isOperator())
                      if (op.isOperator())
                        if ((op.asOperator() == FormulaToken::OperatorAdd)
                                || (op.asOperator() == FormulaToken::OperatorSub)) {
                        ruleFound = true;
                        syntaxStack.pop();
                        syntaxStack.pop();
                        syntaxStack.push(x);
                        if (op.asOperator() == FormulaToken::OperatorSub)
                            m_codes.append(Opcode(Opcode::Neg));
                    }
                }

                if (!ruleFound)
                    break;
            }

            syntaxStack.push(token);
        }
    }

    // syntaxStack must left only one operand and end-of-formula (i.e. InvalidOp)
    m_valid = false;
    if (syntaxStack.itemCount() == 2)
      if (syntaxStack.top().isOperator())
        if (syntaxStack.top().asOperator() == FormulaToken::OperatorInvalid)
          if (!syntaxStack.top(1).isOperator())
              m_valid = true;

    // bad parsing ? clean-up everything
    if (! m_valid) {
        m_constants.clear();
        m_codes.clear();
        kWarning() << "compiling of "<< m_text << " failed";
    }

    return m_valid;
}

QString EnhancedPathFormula::toString() const
{
    return m_text;
}


FormulaToken::FormulaToken(Type type, const QString &text, int position)
    : m_type(type), m_text(text), m_position(position)
{
}

FormulaToken::FormulaToken(const FormulaToken &token)
{
    if (this != &token)
        *this = token;
}

FormulaToken & FormulaToken::operator=(const FormulaToken &rhs)
{
    if (this == &rhs)
        return *this;

    m_type = rhs.m_type;
    m_text = rhs.m_text;
    m_position = rhs.m_position;

    return *this;
}

qreal FormulaToken::asNumber() const
{
    if (isNumber())
        return m_text.toDouble();
    else
        return 0.0;
}

FormulaToken::Operator FormulaToken::asOperator() const
{
    if (isOperator())
        return matchOperator(m_text);
    else
        return OperatorInvalid;
}

// helper function: return operator of given token text
FormulaToken::Operator matchOperator(const QString &text)
{
    if (text.length() != 1)
        return FormulaToken::OperatorInvalid;

    QChar c(text[0]);
    switch(c.toAscii()) {
    case '+': return FormulaToken::OperatorAdd; break;
    case '-': return FormulaToken::OperatorSub; break;
    case '*': return FormulaToken::OperatorMul; break;
    case '/': return FormulaToken::OperatorDiv; break;
    case '(': return FormulaToken::OperatorLeftPar; break;
    case ')': return FormulaToken::OperatorRightPar; break;
    case ',': return FormulaToken::OperatorComma; break;
    default : return FormulaToken::OperatorInvalid; break;
    }
}

// helper function: return true for valid identifier character
bool isIdentifier(QChar ch)
{
    return (ch.unicode() == '?') || (ch.unicode() == '$') || (ch.isLetter());
}

// helper function: give operator precedence
// e.g. '+' is 1 while '*' is 3
int opPrecedence(FormulaToken::Operator op)
{
    int prec = -1;
    switch(op) {
    case FormulaToken::OperatorMul: prec = 5; break;
    case FormulaToken::OperatorDiv: prec = 6; break;
    case FormulaToken::OperatorAdd: prec = 3; break;
    case FormulaToken::OperatorSub: prec = 3; break;
    case FormulaToken::OperatorComma: prec = 0; break;
    case FormulaToken::OperatorRightPar: prec = 0; break;
    case FormulaToken::OperatorLeftPar: prec = -1; break;
    default: prec = -1; break;
    }
    return prec;
}

EnhancedPathFormula::Function matchFunction(const QString &text)
{
    if (text == "abs")
        return EnhancedPathFormula::FunctionAbs;
    if (text == "sqrt")
        return EnhancedPathFormula::FunctionSqrt;
    if (text == "sin")
        return EnhancedPathFormula::FunctionSin;
    if (text == "cos")
        return EnhancedPathFormula::FunctionCos;
    if (text == "tan")
        return EnhancedPathFormula::FunctionTan;
    if (text == "atan")
        return EnhancedPathFormula::FunctionAtan;
    if (text == "atan2")
        return EnhancedPathFormula::FunctionAtan2;
    if (text == "min")
        return EnhancedPathFormula::FunctionMin;
    if (text == "max")
        return EnhancedPathFormula::FunctionMax;
    if (text == "if")
        return EnhancedPathFormula::FunctionIf;

    return EnhancedPathFormula::FunctionUnknown;
}

QString matchFunction(EnhancedPathFormula::Function function)
{
    switch(function) {
    case EnhancedPathFormula::FunctionAbs:
        return "fabs";
        break;
    case EnhancedPathFormula::FunctionSqrt:
        return "sqrt";
        break;
    case EnhancedPathFormula::FunctionSin:
        return "sin";
        break;
    case EnhancedPathFormula::FunctionCos:
        return "cos";
        break;
    case EnhancedPathFormula::FunctionTan:
        return "tan";
        break;
    case EnhancedPathFormula::FunctionAtan:
        return "atan";
        break;
    case EnhancedPathFormula::FunctionAtan2:
        return "atan2";
        break;
    case EnhancedPathFormula::FunctionMin:
        return "min";
        break;
    case EnhancedPathFormula::FunctionMax:
        return "max";
        break;
    case EnhancedPathFormula::FunctionIf:
        return "if";
        break;
    default:
        break;
    }

    return "unknown";
}

void EnhancedPathFormula::debugTokens(const TokenList &tokens)
{
#ifndef NDEBUG
    for (int i = 0; i < tokens.count(); i++)
        kDebug() << tokens[i].text();
#else
    Q_UNUSED(tokens);
#endif
}

void EnhancedPathFormula::debugOpcodes()
{
#ifndef NDEBUG
    foreach (const Opcode &c, m_codes) {
        QString ctext;
        switch(c.type) {
        case Opcode::Load: ctext = QString("Load #%1").arg(c.index); break;
        case Opcode::Ref: ctext = QString("Ref #%1").arg(c.index); break;
        case Opcode::Function: ctext = QString("Function (%1)").arg(c.index); break;
        case Opcode::Add: ctext = "Add"; break;
        case Opcode::Sub: ctext = "Sub"; break;
        case Opcode::Mul: ctext = "Mul"; break;
        case Opcode::Div: ctext = "Div"; break;
        case Opcode::Neg: ctext = "Neg"; break;
        default: ctext = "Unknown"; break;
        }
        kDebug() << ctext;
    }
#endif
}
