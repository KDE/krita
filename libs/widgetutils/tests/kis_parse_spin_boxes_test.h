/*
 *  SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
