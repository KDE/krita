/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_PSD_LAYER_STYLE_RESOURCE_H
#define KIS_PSD_LAYER_STYLE_RESOURCE_H

#include <resources/KoResource.h>
#include <QVector>

#include <kritaui_export.h>
#include "kis_psd_layer_style.h"

/**
 * @brief The KisPSDLayerStyleResource class represents an ASL file type resource.
 */
class KRITAUI_EXPORT KisPSDLayerStyleCollectionResource : public KoResource
{
public:
    typedef QVector<KisPSDLayerStyleSP> StylesVector;

public:
    explicit KisPSDLayerStyleCollectionResource(const QString &filename);
    virtual ~KisPSDLayerStyleCollectionResource();

    virtual bool load();
    virtual bool loadFromDevice(QIODevice *dev);

    virtual bool save();
    virtual bool saveToDevice(QIODevice *dev) const;

    virtual QString defaultFileExtension() const;

    StylesVector layerStyles() const;
    void setLayerStyles(StylesVector styles);

    void collectAllLayerStyles(KisNodeSP root);
    void assignAllLayerStyles(KisNodeSP root);

protected:

    virtual QByteArray generateMD5() const;

private:
    StylesVector m_layerStyles;

};


#endif // KIS_PSD_LAYER_STYLE_RESOURCE_H
