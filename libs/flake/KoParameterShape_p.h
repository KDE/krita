/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOPARAMETERSHAPE_P_H
#define KOPARAMETERSHAPE_P_H

#include "kritaflake_export.h"
#include <KoParameterShape.h>

#include <QList>
#include <QPointF>
#include <QSharedData>

class KoParameterShape;

class KRITAFLAKE_EXPORT KoParameterShape::Private : public QSharedData
{
public:
    explicit Private();
    explicit Private(const KoParameterShape::Private &rhs);
    virtual ~Private() = default;

    bool parametric;

    /// the handles that the user can grab and change
    QList<QPointF> handles;
};

#endif
