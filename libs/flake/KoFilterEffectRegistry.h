/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KOFILTEREFFECTREGISTRY_H
#define KOFILTEREFFECTREGISTRY_H

#include <KoGenericRegistry.h>
#include <KoFilterEffectFactoryBase.h>

#include "kritaflake_export.h"

#include <QDomDocument>
class KoFilterEffectLoadingContext;
class KoFilterEffect;

class KRITAFLAKE_EXPORT KoFilterEffectRegistry : public KoGenericRegistry<KoFilterEffectFactoryBase*>
{
public:
    KoFilterEffectRegistry();
    ~KoFilterEffectRegistry() override;

    /**
    * Return the only instance of KoFilterEffectRegistry.
    * Creates an instance on the first call.
    */
    static KoFilterEffectRegistry *instance();

    /**
     * Creates filter effect from given xml element.
     * @param element the xml element to load from
     * @param context the loading context
     * @return the created filter effect if successful, otherwise returns 0
     */
    KoFilterEffect *createFilterEffectFromXml(const QDomElement &element, const KoFilterEffectLoadingContext &context);

private:

    KoFilterEffectRegistry(const KoFilterEffectRegistry&);
    KoFilterEffectRegistry operator=(const KoFilterEffectRegistry&);
    void init();

    class Private;
    Private * const d;
};

#endif // KOFILTEREFFECTREGISTRY_H
