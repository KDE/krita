/*
 *  SPDX-FileCopyrightText: 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KIS_MIMEDATA_H
#define KIS_MIMEDATA_H

#include <QMimeData>
#include <QRect>

#include <kis_types.h>
#include <kritaui_export.h>

class KisShapeController;
class KisNodeDummy;
class KisNodeInsertionAdapter;
class KisNodeGraphListener;
class KisProcessingApplicator;
class KisDisplayConfig;

/**
 * KisMimeData implements delayed retrieval of nodes for d&d and copy/paste.
 *
 * TODO: implement support for the ora format.
 */
class KRITAUI_EXPORT KisMimeData : public QMimeData
{
    Q_OBJECT
public:
    KisMimeData(QList<KisNodeSP> nodes, KisImageSP image, bool forceCopy = false);

    /// return the node set on this mimedata object -- for internal use
    QList<KisNodeSP> nodes() const;

    /**
     * For Cut/Copy/Paste operations we should detach the contents of
     * the mime data from the actual image because the user can modify
     * our image between the Copy/Cut and Paste calls. So we just copy
     * all our nodes into the internal array.
     *
     * It also fixes the problem of Cutting group layers. If we don't copy
     * the node and all its children, it'll be deleted by the Cut operation
     * and we will not be able to paste it correctly later.
     */
    void deepCopyNodes();

    /**
     * KisMimeData provides the following formats if a node has been set:
     * <ul>
     * <li>application/x-krita-node-internal-pointer: requests a pointer to a Krita node.
     * <li>application/x-qt-image: fallback for other applications, returns a QImage of the
     * current node's paintdevice
     * <li>application/zip: allows drop targets that can handle zip files to open the data
     * </ul>
     */
    QStringList formats() const override;

    static KisNodeList loadNodesFast(
        const QMimeData *data,
        KisImageSP image,
        KisShapeController *shapeController,
        bool &copyNode);

    static KisNodeList loadNodesFastAndRecenter(const QPoint &preferredCenter,
            const QMimeData *data,
            KisImageSP image,
            KisShapeController *shapeController,
            bool &copyNode);

    /**
     * Conversion options for mime clips when they are pasted into
     * clipboard or fetched from clipboard (with user selecting
     * "display" space)
     *
     * On unmanaged compositors they should coinside with the
     * space of the display. On managed ones they should be set
     * to the default color space, e.g. sRGB.
     */
    static KisDisplayConfig displayConfigForMimePastes();

private:
    /**
     * Loads a node from a mime container
     * Supports image and color types.
     */
    static KisNodeList loadNonNativeNodes(const QMimeData *data,
                                          KisImageWSP image);

    /**
     * Try load the node, which belongs to the same Krita instance,
     * that is can be fetched without serialization
     */
    static KisNodeList tryLoadInternalNodes(const QMimeData *data,
                                                 KisImageSP image,
                                                 KisShapeController *shapeController,
                                                 bool /* IN-OUT */ &copyNode);

public:
    static QMimeData* mimeForLayers(const KisNodeList &nodes, KisImageSP image, bool forceCopy = false);
    static QMimeData* mimeForLayersDeepCopy(const KisNodeList &nodes, KisImageSP image, bool forceCopy);
    static bool insertMimeLayers(const QMimeData *data,
                                 KisImageSP image,
                                 KisShapeController *shapeController,
                                 KisNodeDummy *parentDummy,
                                 KisNodeDummy *aboveThisDummy,
                                 bool copyNode,
                                 KisNodeInsertionAdapter *nodeInsertionAdapter,
                                 bool changeOffset = false,
                                 QPointF offset = QPointF(),
                                 KisProcessingApplicator *applicator = nullptr);

protected:
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QVariant retrieveData(const QString &mimetype, QVariant::Type preferredType) const override;
#else
    QVariant retrieveData(const QString &mimetype, QMetaType preferredType) const override;
#endif

private:
    static void initializeExternalNode(KisNodeSP *nodes,
                                       KisImageSP srcImage, KisImageSP dstImage,
                                       KisShapeController *shapeController);

private:

    QList<KisNodeSP> m_nodes;
    bool m_forceCopy;
    KisImageSP m_image;
    QRect m_copiedBounds;
};

#endif // KIS_MIMEDATA_H
