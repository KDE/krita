/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QMap>
#include <QString>
#include <KSeExpr/Expression.h>

class SeExprVariable : public KSeExpr::ExprVarRef
{
public:
    double m_value;

    SeExprVariable();
    SeExprVariable(const double v);

    void eval(double *result) override;
    void eval(const char **result) override;
};
