/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisNumParser.h"

double KisNumParser::parseSimpleMathExpr(QString const &expr)
{
    bool ok = false;
    const double ret = KisNumericParser::parseSimpleMathExpr(expr, &ok);
    if (!ok) {
        return qQNaN();
    }
    return ret;
}
