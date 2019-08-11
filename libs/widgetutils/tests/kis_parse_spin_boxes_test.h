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

#ifndef KISPARSESPINBOXESTEST_H
#define KISPARSESPINBOXESTEST_H

#include <QObject>
#include <QStringList>

class KisParseSpinBoxesTest : public QObject
{
    Q_OBJECT

public:
    explicit KisParseSpinBoxesTest();

private Q_SLOTS:

    void testDoubleParseNormal();
    void testDoubleParseProblem();
    void testDoubleParseWithSuffix();
    void testDoubleParseWithPrefix();
    void testIntParseNormal();
    void testIntParseProblem();
    void testIntParseWithSuffix();
    void testIntParseWithPrefix();

private:

    const static QStringList doubleExprs;
    const static QStringList doubleWrongExprs;
    const static QStringList intExprs;
    const static QStringList intWrongExprs;
};

#endif // KISPARSESPINBOXESTEST_H
