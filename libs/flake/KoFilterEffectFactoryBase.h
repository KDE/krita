/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KOFILTEREFFECTFACTORY_H
#define KOFILTEREFFECTFACTORY_H

#include "kritaflake_export.h"

class KoFilterEffect;
class KoFilterEffectConfigWidgetBase;
class QString;

/// API docs go here
class KRITAFLAKE_EXPORT KoFilterEffectFactoryBase
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
