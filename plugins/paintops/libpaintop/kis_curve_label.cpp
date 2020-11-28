/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "kis_curve_label.h"

#include <QString>
#include <QImage>

struct KisCurveLabel::Private {
    QString name;
    QImage icon;
};

KisCurveLabel::KisCurveLabel() : d(new Private)
{
    d->name = "xxx UNDEFINED xxx";
}

KisCurveLabel::KisCurveLabel(const QString& name)
    : d(new Private)
{
    d->name = name;
}
KisCurveLabel::KisCurveLabel(const QImage& icon)
    : d(new Private)
{
    d->icon = icon;
}

KisCurveLabel::KisCurveLabel(const KisCurveLabel& _rhs) : d(new Private(*_rhs.d))
{
}

KisCurveLabel& KisCurveLabel::operator=(const KisCurveLabel& _rhs)
{
    *d = *_rhs.d;
    return *this;
}

KisCurveLabel::~KisCurveLabel()
{
    delete d;
}

QString KisCurveLabel::name() const
{
    return d->name;
}

QImage KisCurveLabel::icon() const
{
    return d->icon;
}
