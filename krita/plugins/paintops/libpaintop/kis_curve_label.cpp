/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_curve_label.h"

#include <QString>
#include <QImage>

struct KisCurveLabel::Private
{
    QString name;
    QImage icon;
};

KisCurveLabel::KisCurveLabel() : d(new Private)
{
    d->name = "xxx UNDEFINED xxx";
}

KisCurveLabel::KisCurveLabel(const QString& name) : d(new Private)
{
    d->name = name;
}
KisCurveLabel::KisCurveLabel(const QImage& icon) : d(new Private)
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
