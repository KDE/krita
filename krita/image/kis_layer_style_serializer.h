/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_LAYER_STYLE_SERIALIZER_H
#define __KIS_LAYER_STYLE_SERIALIZER_H

class QIODevice;

#include "krita_export.h"

#include "kis_external_factory_base.h"

struct KRITAIMAGE_EXPORT KisLayerStyleSerializer {
    virtual ~KisLayerStyleSerializer() {}
    virtual void saveToDevice(QIODevice *device) = 0;
    virtual void readFromDevice(QIODevice *device) = 0;
};

typedef QSharedPointer<KisLayerStyleSerializer> KisLayerStyleSerializerSP;

class KRITAIMAGE_EXPORT KisLayerStyleSerializerFactory
: public KisExternalFactoryBase<KisLayerStyleSerializerSP, KisPSDLayerStyle*,
                                KisLayerStyleSerializerFactory>
{
public:
    static KisLayerStyleSerializerFactory* instance();
};

#endif /* __KIS_LAYER_STYLE_SERIALIZER_H */
