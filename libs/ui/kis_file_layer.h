/*
 *  SPDX-FileCopyrightText: 2013 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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


    ScalingMethod scalingMethod() const;
    void setScalingMethod(ScalingMethod method);

    KisNodeSP clone() const override;
    bool allowAsChild(KisNodeSP) const override;

    bool accept(KisNodeVisitor&) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    KUndo2Command* crop(const QRect & rect) override;
    KUndo2Command* transform(const QTransform &transform) override;

    void setImage(KisImageWSP image) override;

private Q_SLOTS:
    void slotLoadingFinished(KisPaintDeviceSP projection, qreal xRes, qreal yRes, const QSize &size);
    void openFile() const;

Q_SIGNALS:
    void sigRequestOpenFile();

private:
    QString m_basePath;
    QString m_filename;
    ScalingMethod m_scalingMethod;

    KisPaintDeviceSP m_paintDevice;
    KisSafeDocumentLoader m_loader;
};

#endif // KIS_FILE_LAYER_H
