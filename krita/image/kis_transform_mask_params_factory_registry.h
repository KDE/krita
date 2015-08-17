/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_TRANSFORM_MASK_PARAMS_FACTORY_REGISTRY_H
#define __KIS_TRANSFORM_MASK_PARAMS_FACTORY_REGISTRY_H

#include <QMap>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "kis_types.h"
#include "kritaimage_export.h"


class QDomElement;


typedef boost::function<KisTransformMaskParamsInterfaceSP (const QDomElement &)> KisTransformMaskParamsFactory;
typedef QMap<QString, KisTransformMaskParamsFactory> KisTransformMaskParamsFactoryMap;

class KRITAIMAGE_EXPORT KisTransformMaskParamsFactoryRegistry
{
private:
    KisTransformMaskParamsFactoryRegistry();
    ~KisTransformMaskParamsFactoryRegistry();

public:
    void addFactory(const QString &id, const KisTransformMaskParamsFactory &factory);
    KisTransformMaskParamsInterfaceSP createParams(const QString &id, const QDomElement &e);

    static KisTransformMaskParamsFactoryRegistry* instance();

private:
    KisTransformMaskParamsFactoryMap m_map;
};

#endif /* __KIS_TRANSFORM_MASK_PARAMS_FACTORY_REGISTRY_H */
