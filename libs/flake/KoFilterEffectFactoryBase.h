/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOFILTEREFFECTFACTORY_H
#define KOFILTEREFFECTFACTORY_H

#include "flake_export.h"

class KoFilterEffect;
class KoFilterEffectConfigWidgetBase;

/// API docs go here
class FLAKE_EXPORT KoFilterEffectFactoryBase
{
public:

    /**
    * Create the new factory
    * @param id a string that will be used internally for referencing the filter effect
    * @param name the user visible name of the filter effect this factory creates
    */
    KoFilterEffectFactoryBase(const QString &id, const QString &name);
    virtual ~KoFilterEffectFactoryBase();

    /**
    * Returns the id for the filter this factory creates.
    * @return the id for the filter this factory creates
    */
    QString id() const;

    /**
    * Returns the user visible (and translated) name to be seen by the user.
    * @return the user visible (and translated) name to be seen by the user
    */
    QString name() const;

    /**
    * This method should be implemented by factories to create a filter effect.
    * @return a new filter effect
    */
    virtual KoFilterEffect *createFilterEffect() const = 0;

    /**
     * This method should be implemented by factories to create a filter effect config widget.
     * @return the filter effect options widget
     */
    virtual KoFilterEffectConfigWidgetBase *createConfigWidget() const = 0;

private:
    class Private;
    Private * const d;
};

#endif // KOFILTEREFFECTFACTORY_H
