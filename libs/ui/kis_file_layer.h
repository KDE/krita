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

    KisFileLayer(KisImageWSP image, const QString &name, quint8 opacity);
    /**
     * @brief KisFileLayer create a new file layer with the given file
     * @param image the image the file layer will belong to
     * @param basePath the path to the image, if it has been saved before.
     * @param filename the path to the file, relative to the basePath
     * @param scalingMethod @see ScalingMethod
     * @param name the name of the layer
     * @param opacity the opacity of the layer
     */
    KisFileLayer(KisImageWSP image, const QString& basePath, const QString &filename, ScalingMethod scalingMethod, const QString &name, quint8 opacity);
    ~KisFileLayer() override;
    KisFileLayer(const KisFileLayer& rhs);

    QIcon icon() const override;

    void resetCache() override;

    const KoColorSpace *colorSpace() const override;

    KisPaintDeviceSP original() const override;
    KisPaintDeviceSP paintDevice() const override;
    void setSectionModelProperties(const KisBaseNode::PropertyList &properties) override;
    KisBaseNode::PropertyList sectionModelProperties() const override;

    /**
     * @brief setFileName replace the existing file with a new one
     * @param basePath the path to the image, if it has been saved before.
     * @param filename the path to the file, relative to the basePath
     */
    void setFileName(const QString &basePath, const QString &filename);
    QString fileName() const;
    QString path() const;
    void openFile() const;

    ScalingMethod scalingMethod() const;
    void setScalingMethod(ScalingMethod method);

    KisNodeSP clone() const override;
    bool allowAsChild(KisNodeSP) const override;

    bool accept(KisNodeVisitor&) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    KUndo2Command* crop(const QRect & rect) override;
    KUndo2Command* transform(const QTransform &transform) override;

    void setImage(KisImageWSP image) override;

public Q_SLOTS:
    void slotLoadingFinished(KisPaintDeviceSP projection, int xRes, int yRes);

private:
    QString m_basePath;
    QString m_filename;
    ScalingMethod m_scalingMethod;

    KisPaintDeviceSP m_paintDevice;
    KisSafeDocumentLoader m_loader;
};

#endif // KIS_FILE_LAYER_H
