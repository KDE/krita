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

#include <cstring>
#include <QMap>
#include <QString>
#include <SeExpr2/Expression.h>

#include "SeExprVariable.h"

class SeExprExpressionContext : public SeExpr2::Expression
{
public:
    typedef QMap<std::string, SeExprVariable*> VariableMap;

    VariableMap m_vars;

    SeExprExpressionContext(const QString &expr);

    virtual SeExpr2::ExprVarRef *resolveVar(const std::string &name) const override;
};
