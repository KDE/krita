/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_COLOR_SPACE_REGISTRY_P_H_
#define _KO_COLOR_SPACE_REGISTRY_P_H_

#include <KoGenericRegistry.h>

#include "pigment_export.h"

class KoColorSpace;
class KoColorTransformationFactory;

/**
 * This class list the available transformation. The only reason to use directly
 * that class is for adding new factory use the static method
 * KoColorTransformationFactoryRegistry::add.
 */
class PIGMENTCMS_EXPORT KoColorTransformationFactoryRegistry : private KoGenericRegistry<KoColorTransformationFactory*>
{
    friend class KoColorSpace;
public:
    ~KoColorTransformationFactoryRegistry();
    /**
     * Add a KoColorTransformationFactory to the registry.
     */
    static void addColorTransformationFactory(KoColorTransformationFactory* factory);
private:
    static KoColorTransformationFactoryRegistry* instance();
private:
    KoColorTransformationFactoryRegistry();
private:
    struct Private;
    Private* const d;
};

#endif
