/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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

#include "SeExprVariable.h"

SeExprVariable::SeExprVariable()
    : SeExpr2::ExprVarRef(SeExpr2::ExprType().FP(1).Varying())
    , m_value(0)
{
}

SeExprVariable::SeExprVariable(const double v)
    : SeExpr2::ExprVarRef(SeExpr2::ExprType().FP(1).Varying())
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
