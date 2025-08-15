/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISNUMPARSER_H
#define KISNUMPARSER_H

#include <QObject>
#include <QString>
#include <QQmlEngine>

#include <limits>

#include <kis_num_parser.h>

class KisNumParser : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    //! \brief parse an expression to a double.
    Q_INVOKABLE static double parseSimpleMathExpr(QString const &expr);
};

#endif
