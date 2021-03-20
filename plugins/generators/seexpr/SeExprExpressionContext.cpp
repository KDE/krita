/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SeExprExpressionContext.h"

SeExprExpressionContext::SeExprExpressionContext(const QString &expr)
    : KSeExpr::Expression(expr.toStdString())
    , m_vars(VariableMap())
{
}

KSeExpr::ExprVarRef *SeExprExpressionContext::resolveVar(const std::string &name) const
{
    return m_vars.value(name, nullptr);
}
