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

#ifndef _KIS_DYNAMIC_TRANSFORMABLE_FACTORY_H_
#define _KIS_DYNAMIC_TRANSFORMABLE_FACTORY_H_

class KisDynamicTransformable;
class QString;
#include <QWidget>

class KisDynamicTransformableConfigurationWidget : public QWidget
{
public:
    KisDynamicTransformableConfigurationWidget(QWidget* parent);
    ~KisDynamicTransformableConfigurationWidget();
    virtual KisDynamicTransformable* createTransformable() = 0;
    virtual void setTransformable(KisDynamicTransformable*) = 0;
private:
    struct Private;
    Private* const d;
};


class KisDynamicTransformableFactory
{
public:
    KisDynamicTransformableFactory(const QString& _id, const QString& _name);
    virtual ~KisDynamicTransformableFactory();
    const QString& id() const;
    const QString& name() const;
    virtual KisDynamicTransformableConfigurationWidget* createTransformableConfigurationWidget(QWidget* parent) = 0;
private:
    struct Private;
    Private* const d;
};

#endif
