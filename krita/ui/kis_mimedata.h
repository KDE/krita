/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#ifndef KIS_MIMEDATA_H
#define KIS_MIMEDATA_H

#include <QMimeData>

#include <kis_types.h>
#include <krita_export.h>

class KisShapeController;

/**
 * KisMimeData implements delayed retrieval of nodes for d&d and copy/paste.
 *
 * TODO: implement support for the ora format.
 */
class KRITAUI_EXPORT KisMimeData : public QMimeData
{
    Q_OBJECT
public:
    KisMimeData(QList<KisNodeSP> nodes);

    /// return the node set on this mimedata object -- for internal use
    QList<KisNodeSP> nodes() const;

    /**
     * KisMimeData provides the following formats if a node has been set:
     * <ul>
     * <li>application/x-krita-node: requests a whole serialized node. For d&d between instances of Krita.
     * <li>application/x-qt-image: fallback for other applications, returns a QImage of the
     * current node's paintdevice
     * <li>application/zip: allows drop targets that can handle zip files to open the data
     * </ul>
     */
    QStringList formats() const;

    /**
     * Try load the node, which belongs to the same Krita instance,
     * that is can be fetched without serialization
     */
    static QList<KisNodeSP> tryLoadInternalNodes(const QMimeData *data,
                                                 KisImageWSP image,
                                                 KisShapeController *shapeController,
                                                 bool /* IN-OUT */ &copyNode);

    /**
     * Loads a node from a mime container
     * Supports application/x-krita-node and image types.
     */
    static QList<KisNodeSP> loadNodes(const QMimeData *data,
                                      const QRect &imageBounds,
                                      const QPoint &preferredCenter,
                                      bool forceRecenter,
                                      KisImageWSP image,
                                      KisShapeController *shapeController);

protected:

    QVariant retrieveData(const QString &mimetype, QVariant::Type preferredType) const;

private:
    static void initializeExternalNode(KisNodeSP &nodes,
                                       KisImageWSP image,
                                       KisShapeController *shapeController);

private:

    QList<KisNodeSP> m_nodes;

};

#endif // KIS_MIMEDATA_H
