/*
 *  SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSIMPLEMATHPARSERTEST_H
#define KISSIMPLEMATHPARSERTEST_H

#include <QObject>

class KisSimpleMathParserTest : public QObject
{
    Q_OBJECT

public:
    KisSimpleMathParserTest();

private Q_SLOTS:
    void testDoubleComputation();
    void testIntComputation();
    void testIntFlooring();
};

#endif // KISSIMPLEMATHPARSERTEST_H
