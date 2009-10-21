/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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


#include "kis_dynamic_transformable_factory.h"

struct KisDynamicTransformableConfigurationWidget::Private {
};

KisDynamicTransformableConfigurationWidget::KisDynamicTransformableConfigurationWidget(QWidget* parent) : QWidget(parent), d(new Private)
{
}

KisDynamicTransformableConfigurationWidget::~KisDynamicTransformableConfigurationWidget()
{
    delete d;
}

struct KisDynamicTransformableFactory::Private {
    QString id;
    QString name;
};

KisDynamicTransformableFactory::KisDynamicTransformableFactory(const QString& _id, const QString& _name) : d(new Private)
{
    d->id = _id;
    d->name = _name;
}

KisDynamicTransformableFactory::~KisDynamicTransformableFactory()
{
    delete d;
}

const QString& KisDynamicTransformableFactory::id() const
{
    return d->id;
}

const QString& KisDynamicTransformableFactory::name() const
{
    return d->name;
}
