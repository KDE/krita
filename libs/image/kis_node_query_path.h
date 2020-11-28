/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_NODE_QUERY_PATH_H_
#define _KIS_NODE_QUERY_PATH_H_

#include <kis_types.h>
#include <kritaimage_export.h>

/**
 * This class represent a path to access a node starting from an other node.
 */
class KRITAIMAGE_EXPORT KisNodeQueryPath
{
    KisNodeQueryPath();
public:
    ~KisNodeQueryPath();
    KisNodeQueryPath(const KisNodeQueryPath&);
    KisNodeQueryPath& operator=(const KisNodeQueryPath&);
    QList<KisNodeSP> queryNodes(KisImageWSP image, KisNodeSP currentNode) const;
    KisNodeSP queryUniqueNode(KisImageWSP image, KisNodeSP currentNode = 0) const;
    bool isRelative() const;
    // Use "///" style because of the needed "/*"
    /// This function return a string representing this path. Which is a list separated by '\' of:
    /// - '*': represents all layers
    /// - '..': represents the parent layer
    /// - number: index of the layer
    /// - '.': represents the current layer
    ///
    /// For instance: "1/*" return all children of the first layer, "../3" return the third layer of the parent
    /// of the current layer
    /// If the string starts with "/" then it's an absolute path, otherwise it's a relative path.
    QString toString() const;
    /**
     * @param path
     * @param err if non null, it will be filled with an error message
     * @see toString for an explanation of the string format
     */
    static KisNodeQueryPath fromString(const QString& path);
    static KisNodeQueryPath absolutePath(KisNodeSP node);
private:
    struct Private;
    Private* const d;
};

#endif
