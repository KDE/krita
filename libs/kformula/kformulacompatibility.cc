/* This file is part of the KDE project
   Copyright (C) 2001 Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

   This file is based on the other kformula lib
   Copyright (C) 1999 Ilya Baran (ibaran@mit.edu)

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

#include <q3valuelist.h>

#include <kdebug.h>
#include "kformuladefs.h"
#include "kformulacompatibility.h"

KFORMULA_NAMESPACE_BEGIN

const int SYMBOL_ABOVE = 20000;
const int UNUSED_OFFSET = 1000;

typedef int BoxType;

//const BoxType PLUS = '+';
//const BoxType MINUS = '-';
//const BoxType TIMES = '*';
const BoxType OF_DIVIDE = '\\' + UNUSED_OFFSET;
const BoxType OF_POWER  = '^' + UNUSED_OFFSET; //just a test to see if it works
const BoxType OF_SQRT = '@' + UNUSED_OFFSET;
//const BoxType TEXT = 't';
//const BoxType CAT = '#' + UNUSED_OFFSET;
const BoxType OF_SUB = '_' + UNUSED_OFFSET;
const BoxType OF_LSUP = '6' + UNUSED_OFFSET;
const BoxType OF_LSUB = '%' + UNUSED_OFFSET;
//const BoxType PAREN = '(';
//const BoxType EQUAL = '=';
//const BoxType MORE = '>';
//const BoxType LESS = '<';
//const BoxType ABS = '|';
//const BoxType BRACKET = '[';
//const BoxType SLASH = '/';
const BoxType OF_MATRIX = 'm' + UNUSED_OFFSET;
const BoxType OF_SEPARATOR = '&' + UNUSED_OFFSET; // separator for matrices
const BoxType OF_ABOVE = ')' + UNUSED_OFFSET; //something useless
const BoxType OF_BELOW = ']' + UNUSED_OFFSET;
const BoxType OF_SYMBOL = 's' + UNUSED_OFFSET; // whatever
// char for keeping track of cursor position in undo/redo:
//const BoxType CURSOR = 'c' + UNUSED_OFFSET;

const int INTEGRAL = SYMBOL_ABOVE + 0; // symbols have values above that
const int SUM      = SYMBOL_ABOVE + 1;
const int PRODUCT  = SYMBOL_ABOVE + 2;
const int ARROW    = SYMBOL_ABOVE + 3;
// elements of the symbol font are their own codes + SYMBOL_ABOVE


Compatibility::Compatibility()
{
}


QDomDocument Compatibility::buildDOM(const QString & text)
{
    QDomDocument doc("KFORMULA");
    pos = 0;
    formulaString = text;
    QDomElement formula = readSequence(doc);
    formula.setTagName("FORMULA");
    doc.appendChild(formula);
    return doc;
}


void Compatibility::appendNextSequence(const QDomDocument& doc, QDomElement element)
{
    if (hasNext() && nextToken() == '{') {
        element.appendChild(readSequence(doc));
    }
    else {
        pushback();
        element.appendChild(doc.createElement("SEQUENCE"));
    }
}


QDomElement Compatibility::getLastSequence(const QDomDocument& doc, QDomElement sequence)
{
    if (sequence.lastChild().nodeName() == "SEQUENCE") {
        QDomNode child = sequence.removeChild(sequence.lastChild());
        return child.toElement();
    }
    else {
        QDomElement newSeq = doc.createElement("SEQUENCE");
        if (!sequence.lastChild().isNull()) {
            QDomNode child = sequence.removeChild(sequence.lastChild());
            newSeq.appendChild(child);
        }
        return newSeq;
    }
}


QDomElement Compatibility::findIndexNode(const QDomDocument& doc, QDomElement sequence)
{
    QDomElement element;
    if (sequence.lastChild().nodeName() == "INDEX") {
        element = sequence.lastChild().toElement();
    }
    else {
        element = doc.createElement("INDEX");
        QDomElement con = doc.createElement("CONTENT");
        element.appendChild(con);
        con.appendChild(getLastSequence(doc, sequence));
        sequence.appendChild(element);
    }
    return element;
}


void Compatibility::appendToSequence(QDomElement sequence, QDomElement element, int leftIndexSeen)
{
    if (leftIndexSeen > 0) {
        if (sequence.lastChild().nodeName() == "INDEX") {
            QDomElement index = sequence.lastChild().toElement();
            if ((index.firstChild().nodeName() == "CONTENT") &&
                (index.firstChild().firstChild().nodeName() == "SEQUENCE")) {
                QDomElement seq = index.firstChild().firstChild().toElement();
                if (element.nodeName() == "SEQUENCE") {
                    index.firstChild().replaceChild(element, seq);
                }
                else {
                    seq.appendChild(element);
                }
                return;
            }
        }
    }
    sequence.appendChild(element);
}


QDomElement Compatibility::readMatrix(const QDomDocument& doc)
{
    QDomElement element = doc.createElement("MATRIX");

    uint cols = nextToken();
    nextToken();
    uint rows = nextToken();

    element.setAttribute("ROWS", rows);
    element.setAttribute("COLUMNS", cols);

    if ((nextToken() == '}') && (nextToken() == OF_MATRIX) && (nextToken() == '{')) {
        Q3ValueList<QDomElement> matrix;
        for (uint c = 0; c < cols; c++) {
            for (uint r = 0; r < rows; r++) {
                if (hasNext() && (nextToken() == '{')) {
                    QDomElement tmp = readSequence(doc);
                    matrix.append(tmp);
                }
                if (hasNext() && (nextToken() != OF_SEPARATOR)) {
                    pushback();
                }
            }
        }
        if (hasNext() && (nextToken() != '}')) {
            pushback();
        }

        if (matrix.count() == rows*cols) {
            for (uint r = 0; r < rows; r++) {
                for (uint c = 0; c < cols; c++) {
                    element.appendChild(matrix[c*rows+r]);
                }
            }
        }
    }
    else {
        pushback();
    }

    return element;
}


QDomElement Compatibility::readSequence(const QDomDocument& doc)
{
    // matrizes start with something that isn't a sequence
    if ((tokenLeft() > 6) && (lookAhead(1) == OF_SEPARATOR)) {
        return readMatrix(doc);
    }

    int leftIndexSeen = 0;
    QDomElement sequence = doc.createElement("SEQUENCE");

    while (hasNext()) {
        ushort ch = nextToken();

        // Debug
        //cout << "read: " << ch << " (" << static_cast<char>(ch) << ')' << endl;

        if (leftIndexSeen > 0) leftIndexSeen--;

        switch (ch) {
            case '{':
                appendToSequence(sequence, readSequence(doc), leftIndexSeen);
                break;
            case '}':
                return sequence;
            case '(':
            case '[':
            case '|': {
                // There is an empty sequence we have to remove
                if (!sequence.lastChild().isNull()) {
                    sequence.removeChild(sequence.lastChild());
                }

                QDomElement element = doc.createElement("BRACKET");
                appendToSequence(sequence, element, leftIndexSeen);
                element.setAttribute("LEFT", ch);
                element.setAttribute("RIGHT", (ch=='(') ? ')' : ((ch=='[') ? ']' : '|'));

                QDomElement con = doc.createElement("CONTENT");
                element.appendChild(con);
                appendNextSequence(doc, con);
                break;
            }
            case OF_DIVIDE: {
                QDomElement element = doc.createElement("FRACTION");

                QDomElement num = doc.createElement("NUMERATOR");
                element.appendChild(num);
                num.appendChild(getLastSequence(doc, sequence));

                QDomElement den = doc.createElement("DENOMINATOR");
                element.appendChild(den);
                appendNextSequence(doc, den);

                appendToSequence(sequence, element, leftIndexSeen);
                break;
            }
            case OF_SQRT: {
                QDomElement element = doc.createElement("ROOT");
                QDomElement con = doc.createElement("CONTENT");
                element.appendChild(con);
                appendNextSequence(doc, con);

                QDomElement ind = doc.createElement("INDEX");
                element.appendChild(ind);
                ind.appendChild(getLastSequence(doc, sequence));

                appendToSequence(sequence, element, leftIndexSeen);
                break;
            }
            case OF_POWER: {
                QDomElement element = findIndexNode(doc, sequence);
                QDomElement upperRight = doc.createElement("UPPERRIGHT");
                element.appendChild(upperRight);
                appendNextSequence(doc, upperRight);
                break;
            }
            case OF_SUB: {
                QDomElement element = findIndexNode(doc, sequence);
                QDomElement lowerRight = doc.createElement("LOWERRIGHT");
                element.appendChild(lowerRight);
                appendNextSequence(doc, lowerRight);
                break;
            }
            case OF_LSUP: {
                QDomElement upperLeft = doc.createElement("UPPERLEFT");
                upperLeft.appendChild(getLastSequence(doc, sequence));
                QDomElement element;
                if (sequence.lastChild().nodeName() == "INDEX") {
                    element = sequence.lastChild().toElement();
                }
                else {
                    element = doc.createElement("INDEX");
                    QDomElement con = doc.createElement("CONTENT");
                    element.appendChild(con);
                    QDomElement seq = doc.createElement("SEQUENCE");
                    con.appendChild(seq);
                    appendToSequence(sequence, element, leftIndexSeen);
                }
                element.appendChild(upperLeft);
                leftIndexSeen = 2;
                break;
            }
            case OF_LSUB: {
                QDomElement lowerLeft = doc.createElement("LOWERLEFT");
                lowerLeft.appendChild(getLastSequence(doc, sequence));
                QDomElement element;
                if (sequence.lastChild().nodeName() == "INDEX") {
                    element = sequence.lastChild().toElement();
                }
                else {
                    element = doc.createElement("INDEX");
                    QDomElement con = doc.createElement("CONTENT");
                    element.appendChild(con);
                    QDomElement seq = doc.createElement("SEQUENCE");
                    con.appendChild(seq);
                    appendToSequence(sequence, element, leftIndexSeen);
                }
                element.appendChild(lowerLeft);
                leftIndexSeen = 2;
                break;
            }
            case OF_ABOVE: {
                if (sequence.lastChild().nodeName() == "SEQUENCE") {
                    QDomElement seq = sequence.lastChild().toElement();
                    if ((seq.childNodes().count() == 1) &&
                        ((seq.lastChild().nodeName() == "SYMBOL") ||
                         (seq.lastChild().nodeName() == "INDEX"))) {
                        sequence.removeChild(seq);

                        QDomElement element = seq.lastChild().toElement();
                        QDomElement upper = (element.nodeName() == "SYMBOL") ?
                            doc.createElement("UPPER") :
                            doc.createElement("UPPERMIDDLE");
                        element.appendChild(upper);
                        appendNextSequence(doc, upper);
                        appendToSequence(sequence, element, leftIndexSeen);
                        break;
                    }
                }
                QDomElement element = findIndexNode(doc, sequence);
                QDomElement upper = doc.createElement("UPPERMIDDLE");
                element.appendChild(upper);
                appendNextSequence(doc, upper);
                break;
            }
            case OF_BELOW: {
                if (sequence.lastChild().nodeName() == "SEQUENCE") {
                    QDomElement seq = sequence.lastChild().toElement();
                    if ((seq.childNodes().count() == 1) &&
                        ((seq.lastChild().nodeName() == "SYMBOL") ||
                         (seq.lastChild().nodeName() == "INDEX"))) {
                        sequence.removeChild(seq);

                        QDomElement element = seq.lastChild().toElement();
                        QDomElement lower = (element.nodeName() == "SYMBOL") ?
                            doc.createElement("LOWER") :
                            doc.createElement("LOWERMIDDLE");
                        element.appendChild(lower);
                        appendNextSequence(doc, lower);
                        appendToSequence(sequence, element, leftIndexSeen);
                        break;
                    }
                }
                QDomElement element = findIndexNode(doc, sequence);
                QDomElement lower = doc.createElement("LOWERMIDDLE");
                element.appendChild(lower);
                appendNextSequence(doc, lower);
                break;
            }
            case OF_SYMBOL:
                kDebug() << "OF_SYMBOL" << endl;
                break;
            case INTEGRAL:
            case SUM:
            case PRODUCT: {
                QDomElement element = doc.createElement("SYMBOL");
                element.setAttribute("TYPE",
                                     (ch==INTEGRAL) ? Integral :
                                     ((ch==SUM) ? Sum : Product));

                QDomElement con = doc.createElement("CONTENT");
                element.appendChild(con);
                con.appendChild(readSequence(doc));
                pushback();
                appendToSequence(sequence, element, leftIndexSeen);
                break;
            }
            case ARROW: {
                QDomElement element = doc.createElement("TEXT");
                element.setAttribute("CHAR", QString(QChar(static_cast<char>(174))));
                element.setAttribute("SYMBOL", "1");
                appendToSequence(sequence, element, leftIndexSeen);
                break;
            }
            default: {
                QDomElement element = doc.createElement("TEXT");
                element.setAttribute("CHAR", QString(formulaString[pos-1]));
                appendToSequence(sequence, element, leftIndexSeen);
            }
        }
    }
    return sequence;
}

KFORMULA_NAMESPACE_END
