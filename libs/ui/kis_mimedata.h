/*
 *  SPDX-FileCopyrightText: 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KIS_MIMEDATA_H
#define KIS_MIMEDATA_H

#include <QMimeData>

#include <kis_types.h>
#include <kritaui_export.h>

class KisShapeController;
class KisNodeDummy;
class KisNodeInsertionAdapter;
class KisNodeGraphListener;

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
     * <li>application/x-krita-node: requests a whole serialized node. For d&d between instances of Krita.
     * <li>application/x-qt-image: fallback for other applications, returns a QImage of the
     * current node's paintdevice
     * <li>application/zip: allows drop targets that can handle zip files to open the data
     * </ul>
     */
    QStringList formats() const override;

    /**
     * Loads a node from a mime container
     * Supports application/x-krita-node and image types.
     */
    static KisNodeList loadNodes(const QMimeData *data,
                                      const QRect &imageBounds,
                                      const QPoint &preferredCenter,
                                      bool forceRecenter,
                                      KisImageWSP image,
                                      KisShapeController *shapeController);

    static KisNodeList loadNodesFast(
        const QMimeData *data,
        KisImageSP image,
        KisShapeController *shapeController,
        bool &copyNode);

private:
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
                                 KisNodeInsertionAdapter *nodeInsertionAdapter);

protected:

    QVariant retrieveData(const QString &mimetype, QVariant::Type preferredType) const override;

private:
    static void initializeExternalNode(KisNodeSP *nodes,
                                       KisImageWSP image,
                                       KisShapeController *shapeController);

private:

    QList<KisNodeSP> m_nodes;
    bool m_forceCopy;
    KisImageSP m_image;
};

#endif // KIS_MIMEDATA_H
