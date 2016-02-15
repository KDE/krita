/*
 *  Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_FILE_LAYER_H
#define KIS_FILE_LAYER_H

#include "kritaui_export.h"

#include "kis_external_layer_iface.h"
#include "kis_safe_document_loader.h"



/**
 * @brief The KisFileLayer class loads a particular file as a layer into the layer stack.
 */
class KRITAUI_EXPORT KisFileLayer : public KisExternalLayer
{
    Q_OBJECT
public:

    enum ScalingMethod {
        None,
        ToImageSize,
        ToImagePPI
    };

    explicit KisFileLayer(KisImageWSP image, const QString& basePath, const QString &filename, ScalingMethod scalingMethod, const QString &name, quint8 opacity);
    ~KisFileLayer();
    KisFileLayer(const KisFileLayer& rhs);

    void resetCache();

    virtual const KoColorSpace *colorSpace() const;

    KisPaintDeviceSP original() const;
    KisPaintDeviceSP paintDevice() const;
    KisBaseNode::PropertyList sectionModelProperties() const;

    void setFileName(const QString &basePath, const QString &filename);
    QString fileName() const;
    QString path() const;

    ScalingMethod scalingMethod() const;
    
    KisNodeSP clone() const;
    bool allowAsChild(KisNodeSP) const;

    bool accept(KisNodeVisitor&);
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter);

    KUndo2Command* crop(const QRect & rect);
    KUndo2Command* transform(const QTransform &transform);

public Q_SLOTS:
    void slotLoadingFinished(KisImageSP importedImage);

private:
    QString m_basePath;
    QString m_filename;
    ScalingMethod m_scalingMethod;

    KisPaintDeviceSP m_image;
    KisSafeDocumentLoader m_loader;
};

#endif // KIS_FILE_LAYER_H
