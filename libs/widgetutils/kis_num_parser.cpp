/*
 *  SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_num_parser.h"

#include <qnumeric.h> // for qIsNaN
#include <qmath.h>
#include <QVector>
#include <QRegExp>
#include <QStringList>
#include <QVariant>
#include <QLocale>

#include <iostream>

using namespace std;

const QVector<char> opLevel1 = {'+', '-'};
const QVector<char> opLevel2 = {'*', '/'};

const QStringList supportedFuncs = {"", "cos", "sin", "tan", "acos", "asin", "atan", "exp", "ln", "log10", "abs"};

const QRegExp funcExpr("(-)?([a-zA-Z]*[0-9]*)?\\((.+)\\)");
const QRegExp numberExpr("(-)?([0-9]+\\.?[0-9]*(e[0-9]*)?)");

const QRegExp funcExprInteger("(-)?\\((.+)\\)");
const QRegExp integerExpr("(-)?([0-9]+)");

//double functions
double treatFuncs(QString const& expr, bool & noProblem);
double treatLevel1(QString const& expr, bool & noProblem);
double treatLevel2(QString const& expr, bool & noProblem);
double treatLevel3(QString const& expr, bool & noProblem);

//int functions
double treatLevel1Int(QString const& expr, bool & noProblem);
double treatLevel2Int(QString const& expr, bool & noProblem);
double treatFuncsInt(QString const& expr, bool & noProblem);

namespace KisNumericParser {

/*!
 * \param expr the expression to parse
 * \param noProblem if provided, the value pointed to will be se to true is no problem appeared, false otherwise.
 * \return the numerical value the expression eval to (or 0 in case of error).
 */
double parseSimpleMathExpr(const QString &expr, bool *noProblem)
{

    bool ok = true; //intermediate variable to pass by reference to the sublevel parser (if no pointer is provided).

    //then go down each 3 levels of operation priority.
    if (noProblem != nullptr) {
        return treatLevel1(expr, *noProblem);
    }

    return treatLevel1(expr, ok);

}

/*!
 * \param expr the expression to parse
 * \param noProblem if provided, the value pointed to will be se to true is no problem appeared, false otherwise.
 * \return the numerical value the expression eval to (or 0 in case of error).
 */
int parseIntegerMathExpr(QString const& expr, bool* noProblem)
{

    bool ok = true; //intermediate variable to pass by reference to the sublevel parser (if no pointer is provided).

    if (noProblem != nullptr) {
        return qRound(treatLevel1Int(expr, *noProblem));
    }

    return qRound(treatLevel1Int(expr, ok));

}

} //namespace KisNumericParser.


//intermediate functions

/*!
 * \brief extractSubExprLevel1 extract from an expression the part of an expression that need to be treated recursively before computing level 1 operations (+, -).
 * \param expr The expression to treat, the part returned will be removed.
 * \param nextOp This reference, in case of success, will hold the first level operation identified as separator ('+' or '-')
 * \param noProblem A reference to a bool, set to true if there was no problem, false otherwise.
 * \return The first part of the expression that doesn't contain first level operations not nested within parenthesis.
 */
inline QString extractSubExprLevel1(QString & expr, char & nextOp, bool & noProblem){

    QString ret;

    int subCount = 0;

    bool lastMetIsNumber = false;

    for(int i = 0; i < expr.size(); i++){

    if (expr.at(i) == '(') {
        subCount++;
    }

    if (expr.at(i) == ')') {
        subCount--;
    }

    if (subCount < 0) {
        noProblem = false;
        return ret;
    }

    if(i == expr.size()-1 && subCount == 0){
        ret = expr;
        expr.clear();
        break;
    }

    if( (expr.at(i) == '+' || expr.at(i) == '-') &&
            subCount == 0) {

        if (expr.at(i) == '-' &&
                i < expr.size()-1) {

            bool cond = !expr.at(i+1).isSpace();

            if (cond && !lastMetIsNumber) {
                continue;
            }

        }

        ret = expr.mid(0, i).trimmed();
        nextOp = expr.at(i).toLatin1();
        expr = expr.mid(i+1);
        break;

    }

    if (expr.at(i).isDigit()) {
        lastMetIsNumber = true;
    } else if (expr.at(i) != '.' &&
              !expr.at(i).isSpace()) {
        lastMetIsNumber = false;
    }

    }

    noProblem = true;
    return ret;
}


/*!
 * \brief extractSubExprLevel2 extract from an expression the part of an expression that need to be treated recursively before computing level 2 operations (*, /).
 * \param expr The expression to treat, the part returned will be removed.
 * \param nextOp This reference, in case of success, will hold the first level operation identified as separator ('*' or '/')
 * \param noProblem A reference to a bool, set to true if there was no problem, false otherwise.
 * \return The first part of the expression that doesn't contain second level operations not nested within parenthesis.
 */
inline QString extractSubExprLevel2(QString & expr, char & nextOp, bool & noProblem){

    QString ret;

    int subCount = 0;

    for(int i = 0; i < expr.size(); i++){

    if (expr.at(i) == '(') {
        subCount++;
    }

    if (expr.at(i) == ')') {
        subCount--;
    }

    if (subCount < 0) {
        noProblem = false;
        return ret;
    }

    if(i == expr.size()-1 && subCount == 0){
        ret = expr;
        expr.clear();
        break;
    }

    if( (expr.at(i) == '*' || expr.at(i) == '/') &&
            subCount == 0) {

        ret = expr.mid(0, i).trimmed();
        nextOp = expr.at(i).toLatin1();
        expr = expr.mid(i+1);
        break;

    }

    }

    noProblem = true;
    return ret;
}

/*!
 * \brief treatLevel1 treat an expression at the first level of recursion.
 * \param expr The expression to treat.
 * \param noProblem A reference to a bool set to true if no problem happened, false otherwise.
 * \return The value of the parsed expression or subexpression or 0 in case of error.
 */
double treatLevel1(const QString &expr, bool & noProblem)
{

    noProblem = true;

    QString exprDestructable = expr;

    char nextOp = '+';
    double result = 0.0;

    while (!exprDestructable.isEmpty()) {

        double sign = (nextOp == '-') ? -1 : 1;
        QString part = extractSubExprLevel1(exprDestructable, nextOp, noProblem);

        if (!noProblem) {
        return 0.0;
        }

        if (sign > 0) {
        result += treatLevel2(part, noProblem);
        } else {
        result -= treatLevel2(part, noProblem);
        }

        if(!noProblem){
        return 0.0;
        }
    }

    return result;

}

/*!
 * \brief treatLevel2 treat a subexpression at the second level of recursion.
 * \param expr The subexpression to treat.
 * \param noProblem A reference to a bool set to true if no problem happened, false otherwise.
 * \return The value of the parsed subexpression or 0 in case of error.
 *
 * The expression should not contain first level operations not nested in parenthesis.
 */
double treatLevel2(QString const& expr, bool & noProblem)
{

    noProblem = true;

    QString exprDestructable = expr;

    char nextOp = '*';

    QString part = extractSubExprLevel2(exprDestructable, nextOp, noProblem);

    double result = treatLevel3(part, noProblem);

    while (!exprDestructable.isEmpty()) {

        if (!noProblem) {
        return 0.0;
        }

        bool needToMultiply = (nextOp == '*');
        part = extractSubExprLevel2(exprDestructable, nextOp, noProblem);

        if (!noProblem) {
        return 0.0;
        }

        if (needToMultiply) {
        result *= treatLevel3(part, noProblem);
        } else {
        result /= treatLevel3(part, noProblem);
        }
    }

    return result;
}

/*!
 * \brief treatLevel3 treat a subexpression at the third level of recursion.
 * \param expr The subexpression to treat.
 * \param noProblem A reference to a bool set to true if no problem happened, false otherwise.
 * \return The value of the parsed subexpression or 0 in case of error.
 *
 * The expression should not contain first or second level operations not nested in parenthesis.
 */
double treatLevel3(const QString &expr, bool & noProblem)
{

    noProblem = true;

    int indexPower = -1;
    int indexCount = 0;
    int subLevels = 0;

    for (int i = 0; i < expr.size(); i++) {
        if (expr.at(i) == '(') {
            subLevels++;
        } else if(expr.at(i) == ')') {
            subLevels--;
            if (subLevels < 0) {
                noProblem = false;
                return 0.0;
            }
        } else if (expr.at(i) == '^') {
            if (subLevels == 0) {
                indexPower = i;
                indexCount++;
            }
        }
    }

    if (indexCount > 1 || indexPower + 1 >= expr.size()) {
        noProblem = false;
        return 0.0;
    }

    if (indexPower > -1) {

        QStringList subExprs;
        subExprs << expr.mid(0,indexPower);
        subExprs << expr.mid(indexPower+1);

        bool noProb1 = true;
        bool noProb2 = true;

        double base = treatFuncs(subExprs[0], noProb1);
        double power = treatFuncs(subExprs[1], noProb2);

        return qPow(base, power);

    } else {
        return treatFuncs(expr, noProblem);
    }

    noProblem = false;
    return 0.0;

}

/*!
 * \brief treatFuncs treat the last level of recursion: parenthesis and functions.
 * \param expr The expression to parse.
 * \param noProblem A reference to a bool set to true if no problem happened, false otherwise.
 * \return The value of the parsed subexpression or 0 in case of error.
 *
 * The expression should not contain operators not nested anymore. The subexpressions within parenthesis will be treated by recalling the level 1 function.
 */
double treatFuncs(QString const& expr, bool & noProblem)
{

    noProblem = true;

    QRegExp funcExp = funcExpr; //copy the expression in the current execution stack, to avoid errors for example when multiple thread call this function.
    QRegExp numExp = numberExpr;

    if (funcExp.exactMatch(expr.trimmed())) {

        int sign = funcExp.capturedTexts()[1].isEmpty() ? 1 : -1;
        QString func = funcExp.capturedTexts()[2].toLower();
        QString subExpr = funcExp.capturedTexts()[3];

        double val = treatLevel1(subExpr, noProblem);

        if (!noProblem) {
            return 0.0;
        }

        if (func.isEmpty()) {
            return sign*val;
        }

        if (!supportedFuncs.contains(func)) {
            noProblem = false;
            return 0.0;
        }

        //trigonometry is done in degree
        if (func == "cos") {
            val = qCos(val/180*qAcos(-1));
        } else if (func == "sin") {
            val = qSin(val/180*qAcos(-1));
        } else if (func == "tan") {
            val = qTan(val/180*qAcos(-1));
        } else if(func == "acos") {
            val = qAcos(val)*180/qAcos(-1);
        } else if (func == "asin") {
            val = qAsin(val)*180/qAcos(-1);
        } else if (func == "atan") {
            val = qAtan(val)*180/qAcos(-1);
        } else if (func == "exp") {
            val = qExp(val);
        } else if (func == "ln") {
            val = qLn(val);
        } else if (func == "log10") {
            val = qLn(val)/qLn(10.0);
        } else if (func == "abs") {
            val = qAbs(val);
        }

        return sign*val;
    } else if(numExp.exactMatch(expr.trimmed())) {
        return expr.toDouble(&noProblem);
    }

    double val = QLocale().toDouble(expr, &noProblem);

    if(noProblem) {
        return val;
    }

    noProblem = false;
    return 0.0;

}

//int functions
/*!
 * \brief treatLevel1 treat an expression at the first level of recursion.
 * \param expr The expression to treat.
 * \param noProblem A reference to a bool set to true if no problem happened, false otherwise.
 * \return The value of the parsed expression or subexpression or 0 in case of error.
 */
double treatLevel1Int(QString const& expr, bool & noProblem)
{

    noProblem = true;

    QString exprDestructable = expr;

    char nextOp = '+';
    double result = 0.0;

    while (!exprDestructable.isEmpty()) {

    double sign = (nextOp == '-') ? -1 : 1;
    QString part = extractSubExprLevel1(exprDestructable, nextOp, noProblem);

    if( !noProblem) {
        return 0.0;
    }

    if (sign > 0) {
        result += treatLevel2Int(part, noProblem);
    } else {
        result -= treatLevel2Int(part, noProblem);
    }

    if(!noProblem){
        return 0.0;
    }
    }

    return result;

}

/*!
 * \brief treatLevel2 treat a subexpression at the second level of recursion.
 * \param expr The subexpression to treat.
 * \param noProblem A reference to a bool set to true if no problem happened, false otherwise.
 * \return The value of the parsed subexpression or 0 in case of error.
 *
 * The expression should not contain first level operations not nested in parenthesis.
 */
double treatLevel2Int(const QString &expr, bool &noProblem)
{

    noProblem = true;

    QString exprDestructable = expr;

    char nextOp = '*';

    QString part = extractSubExprLevel2(exprDestructable, nextOp, noProblem);

    double result = treatFuncsInt(part, noProblem);

    while (!exprDestructable.isEmpty()) {

    if (!noProblem) {
        return 0.0;
    }

    bool needToMultiply = (nextOp == '*');
    part = extractSubExprLevel2(exprDestructable, nextOp, noProblem);

    if (!noProblem) {
        return 0.0;
    }

    if (needToMultiply) {
        result *= treatFuncsInt(part, noProblem);
    } else {

        double val = treatFuncsInt(part, noProblem);

        if(std::isinf(result/val) || qIsNaN(result/val)){
        noProblem = false;
        return 0.0;
        }

        result /= val;
    }
    }

    return result;

}

/*!
 * \brief treatFuncs treat the last level of recursion: parenthesis
 * \param expr The expression to parse.
 * \param noProblem A reference to a bool set to true if no problem happened, false otherwise.
 * \return The value of the parsed subexpression or 0 in case of error.
 *
 * The expression should not contain operators not nested anymore. The subexpressions within parenthesis will be treated by recalling the level 1 function.
 */
double treatFuncsInt(QString const& expr, bool & noProblem)
{

    noProblem = true;

    QRegExp funcExpInteger = funcExprInteger;
    QRegExp integerExp = integerExpr;
    QRegExp numberExp = numberExpr;

    if (funcExpInteger.exactMatch(expr.trimmed())) {

        int sign = funcExpInteger.capturedTexts()[1].isEmpty() ? 1 : -1;
        QString subExpr = funcExpInteger.capturedTexts()[2];

        double val = treatLevel1Int(subExpr, noProblem);

        if (!noProblem) {
            return 0;
        }

        return sign*val;

    } else if(numberExp.exactMatch(expr.trimmed())) {
        double value = QVariant(expr).toDouble(&noProblem);
        return value;
    }

    noProblem = false;
    return 0;

}
