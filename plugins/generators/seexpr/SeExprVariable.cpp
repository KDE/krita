/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SeExprVariable.h"

SeExprVariable::SeExprVariable()
    : KSeExpr::ExprVarRef(KSeExpr::ExprType().FP(1).Varying())
    , m_value(0)
{
}

SeExprVariable::SeExprVariable(const double v)
    : KSeExpr::ExprVarRef(KSeExpr::ExprType().FP(1).Varying())
    , m_value(v)
{
}

void SeExprVariable::eval(double *result)
{
    result[0] = m_value;
}

void SeExprVariable::eval(const char **)
{
    Q_ASSERT(!"Sanity check");
}
