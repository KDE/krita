/*
 *  Copyright (c) 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_numparser.h"

//#include <QtMath>
#include <qmath.h>
#include <QVector>
#include <QRegExp>
#include <QStringList>
#include <QVariant>

using namespace std;

const QVector<char> opLevel1 = {'+', '-'};
const QVector<char> opLevel2 = {'*', '/'};

const QStringList supportedFuncs = {"", "cos", "sin", "tan", "acos", "asin", "atan", "exp", "ln", "log10", "abs"};

const QRegExp funcExpr("(-)?([a-zA-Z]*)?\\((.+)\\)");
const QRegExp numberExpr("(-)?([0-9]+\\.?[0-9]*(e[0-9]*)?)");

const QRegExp funcExprInteger("(-)?\\((.+)\\)");
const QRegExp integerExpr("(-)?([0-9]+)");

//double functions
double treatFuncs(QString const& expr, bool & noProblem);
double treatLevel1(QString const& expr, bool & noProblem);
double treatLevel2(QString const& expr, bool & noProblem);
double treatLevel3(QString const& expr, bool & noProblem);

//int functions
int treatLevel1Int(QString const& expr, bool & noProblem);
int treatLevel2Int(QString const& expr, bool & noProblem);
int treatFuncsInt(QString const& expr, bool & noProblem);

namespace KisNumericParser {

double parseSimpleMathExpr(const QString &expr, bool *ok){

        bool oke = true;

        //then go down each 3 levels of operation priority.
        if(ok != nullptr) {
                return treatLevel1(expr, *ok);
        }

        return treatLevel1(expr, oke);

}

int parseIntegerMathExpr(QString const& expr, bool* ok){

        bool oke = true;

        if(ok != nullptr) {
                return treatLevel1Int(expr, *ok);
        }

        return treatLevel1Int(expr, oke);

}

} //namespace utils.


//intermediate functions

inline void cutLevel1(QString const& expr, QStringList & readyToTreat, QVector<char> & op, bool & noProblem){

        readyToTreat.clear();
        op.clear();

        int subCount = 0;
        int lastPos = 0;

        bool lastMetIsNumber = false;

        for(int i = 0; i < expr.size(); i++){

                if(expr.at(i) == '('){
                        subCount++;
                }

                if(expr.at(i) == ')'){
                        subCount--;
                }

                if(subCount < 0){
                        noProblem = false;
                        return;
                }

                if( (expr.at(i) == '+' || expr.at(i) == '-') &&
                                subCount == 0){

                        if(expr.at(i) == '-' &&
                                        i < expr.size()-1){

                                bool cond = !expr.at(i+1).isSpace();

                                if(cond && !lastMetIsNumber){
                                        continue;
                                }

                        }

                        readyToTreat.push_back(expr.mid(lastPos, i-lastPos).trimmed());
                        lastPos = i+1;
                        op.push_back(expr.at(i).toLatin1());

                }

                if(expr.at(i).isDigit()){
                        lastMetIsNumber = true;
                } else if(expr.at(i) != '.' &&
                                  !expr.at(i).isSpace()){
                        lastMetIsNumber = false;
                }
        }

        readyToTreat.push_back(expr.mid(lastPos).trimmed());

}
inline void cutLevel2(QString const& expr, QStringList & readyToTreat, QVector<char> & op, bool & noProblem){

        readyToTreat.clear();
        op.clear();

        int subCount = 0;
        int lastPos = 0;

        for(int i = 0; i < expr.size(); i++){

                if(expr.at(i) == '('){
                        subCount++;
                }

                if(expr.at(i) == ')'){
                        subCount--;
                }

                if(subCount < 0){
                        noProblem = false;
                        return;
                }

                if( (expr.at(i) == '*' || expr.at(i) == '/') &&
                                subCount == 0){

                        readyToTreat.push_back(expr.mid(lastPos, i-lastPos).trimmed());
                        lastPos = i+1;
                        op.push_back(expr.at(i).toLatin1());

                }
        }

        readyToTreat.push_back(expr.mid(lastPos).trimmed());
}

double treatLevel1(const QString &expr, bool & noProblem){

        noProblem = true;

        QStringList readyToTreat;
        QVector<char> op;

        cutLevel1(expr, readyToTreat, op, noProblem);
        if(!noProblem){
                return 0.0;
        }

        if(readyToTreat.contains("")){
                noProblem = false;
                return 0.0;
        }

        if(op.size() != readyToTreat.size()-1){
                noProblem = false;
                return 0.0;
        }

        double result = 0.0;

        for(int i = 0; i < readyToTreat.size(); i++){

                if(i == 0){
                        result += treatLevel2(readyToTreat[i], noProblem);
                } else {
                        if(op[i-1] == '+'){
                                result += treatLevel2(readyToTreat[i], noProblem);
                        } else if(op[i-1] == '-'){
                                result -= treatLevel2(readyToTreat[i], noProblem);
                        }
                }

                if(noProblem == false){
                        return 0.0;
                }
        }

        return result;

}

double treatLevel2(QString const& expr, bool & noProblem){

        noProblem = true;

        QStringList readyToTreat;
        QVector<char> op;

        cutLevel2(expr, readyToTreat, op, noProblem);
        if(!noProblem){
                return 0.0;
        }

        if(readyToTreat.contains("")){
                noProblem = false;
                return 0.0;
        }

        if(op.size() != readyToTreat.size()-1){
                noProblem = false;
                return 0.0;
        }

        double result = 0.0;

        for(int i = 0; i < readyToTreat.size(); i++){

                if(i == 0){
                        result += treatLevel3(readyToTreat[i], noProblem);
                } else {
                        if(op[i-1] == '*'){
                                result *= treatLevel3(readyToTreat[i], noProblem);
                        } else if(op[i-1] == '/'){
                                //may become infinity or NAN.
                                result /= treatLevel3(readyToTreat[i], noProblem);
                        }
                }

                if(noProblem == false){
                        return 0.0;
                }
        }

        return result;
}

double treatLevel3(const QString &expr, bool & noProblem){

        noProblem = true;

        int indexPower = -1;
        int indexCount = 0;
        int subLevels = 0;

        for(int i = 0; i < expr.size(); i++){
                if(expr.at(i) == '('){
                        subLevels++;
                } else if(expr.at(i) == ')'){
                        subLevels--;
                        if(subLevels < 0){
                                noProblem = false;
                                return 0.0;
                        }
                } else if (expr.at(i) == '^'){
                        if(subLevels == 0){
                                indexPower = i;
                                indexCount++;
                        }
                }
        }

        if(indexCount > 1){
                noProblem = false;
                return 0.0;
        }

        if(indexPower > -1){

                QStringList subExprs = expr.split('^');

                bool noProb1 = true;
                bool noProb2 = true;

                double base = treatFuncs(subExprs[0], noProb1);
                double power = treatFuncs(subExprs[1], noProb2);

                return qPow(base, power);

        } else{
                return treatFuncs(expr, noProblem);
        }

        noProblem = false;
        return 0.0;

}

double treatFuncs(QString const& expr, bool & noProblem){

        noProblem = true;

        QRegExp funcExp = funcExpr; //copy the expression in the current execution stack, to avoid errors for example when multiple thread call this function.
        QRegExp numExp = numberExpr;

        if(funcExp.exactMatch(expr.trimmed())){

                int sign = funcExp.capturedTexts()[1].isEmpty() ? 1 : -1;
                QString func = funcExp.capturedTexts()[2].toLower();
                QString subExpr = funcExp.capturedTexts()[3];

                double val = treatLevel1(subExpr, noProblem);

                if(!noProblem){
                        return 0.0;
                }

                if(func.isEmpty()){
                        return sign*val;
                }

                if(!supportedFuncs.contains(func)){
                        noProblem = false;
                        return 0.0;
                }

                //trigonometry is done in degree
                if(func == "cos"){
                        val = qCos(val/180*qAcos(-1));
                } else if (func == "sin"){
                        val = qSin(val/180*qAcos(-1));
                } else if (func == "tan"){
                        val = qTan(val/180*qAcos(-1));
                } else if(func == "acos"){
                        val = qAcos(val)*180/qAcos(-1);
                } else if (func == "asin"){
                        val = qAsin(val)*180/qAcos(-1);
                } else if (func == "atan"){
                        val = qAtan(val)*180/qAcos(-1);
                }else if (func == "exp"){
                        val = qExp(val);
                }else if (func == "ln"){
                        val = qLn(val);
                }else if (func == "log10"){
                        val = qLn(val)/qLn(10.0);
                }else if (func == "abs"){
                        val = qAbs(val);
                }

                return sign*val;
        } else if(numExp.exactMatch(expr.trimmed())){
                return expr.toDouble(&noProblem);
        }

        noProblem = false;
        return 0.0;

}

//int functions
int treatLevel1Int(QString const& expr, bool & noProblem){

        noProblem = true;

        QStringList readyToTreat;
        QVector<char> op;

        cutLevel1(expr, readyToTreat, op, noProblem);
        if(!noProblem){
                return 0.0;
        }

        if(readyToTreat.contains("")){
                noProblem = false;
                return 0;
        }

        if(op.size() != readyToTreat.size()-1){
                noProblem = false;
                return 0;
        }

        int result = 0;

        for(int i = 0; i < readyToTreat.size(); i++){

                if(i == 0){
                        result += treatLevel2Int(readyToTreat[i], noProblem);
                } else {
                        if(op[i-1] == '+'){
                                result += treatLevel2Int(readyToTreat[i], noProblem);
                        } else if(op[i-1] == '-'){
                                result -= treatLevel2Int(readyToTreat[i], noProblem);
                        }
                }

                if(noProblem == false){
                        return 0;
                }
        }

        return result;

}

int treatLevel2Int(const QString &expr, bool &noProblem){

        noProblem = true;

        QStringList readyToTreat;
        QVector<char> op;

        cutLevel2(expr, readyToTreat, op, noProblem);
        if(!noProblem){
                return 0.0;
        }

        if(readyToTreat.contains("")){
                noProblem = false;
                return 0;
        }

        if(op.size() != readyToTreat.size()-1){
                noProblem = false;
                return 0;
        }

        int result = 0;

        for(int i = 0; i < readyToTreat.size(); i++){

                if(i == 0){
                        result += treatFuncsInt(readyToTreat[i], noProblem);
                } else {
                        if(op[i-1] == '*'){
                                result *= treatFuncsInt(readyToTreat[i], noProblem);
                        } else if(op[i-1] == '/'){
                                int value = treatFuncsInt(readyToTreat[i], noProblem);

                                //int int airthmetic it's impossible to divide by 0.
                                if(value == 0){
                                        noProblem = false;
                                        return 0;
                                }

                                result /= value;
                        }
                }

                if(noProblem == false){
                        return 0;
                }
        }

        return result;

}

int treatFuncsInt(QString const& expr, bool & noProblem){

        noProblem = true;

        QRegExp funcExpInteger = funcExprInteger;
        QRegExp integerExp = integerExpr;
        QRegExp numberExp = numberExpr;

        if(funcExpInteger.exactMatch(expr.trimmed())){

                int sign = funcExpInteger.capturedTexts()[1].isEmpty() ? 1 : -1;
                QString subExpr = funcExpInteger.capturedTexts()[2];

                int val = treatLevel1Int(subExpr, noProblem);

                if(!noProblem){
                        return 0;
                }

                return sign*val;

        } else if(integerExp.exactMatch(expr.trimmed())){
                return QVariant(expr).toInt(&noProblem);
        } else if(numberExp.exactMatch(expr.trimmed())){
                double value = qFloor( QVariant(expr).toDouble(&noProblem));
                return (value > 0.0) ? value: value + 1;
        }

        noProblem = false;
        return 0;

}
