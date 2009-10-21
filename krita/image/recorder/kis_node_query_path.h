/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_NODE_QUERY_PATH_H_
#define _KIS_NODE_QUERY_PATH_H_

#include <kis_types.h>
#include <krita_export.h>

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
    bool isRelative() const;
    // Use "///" style because of the needed "/*"
    /// This function return a string representing this path. Which is a list separated by '\' of:
    /// - '*': represents all layers
    /// - '..': represents the parent layer
    /// - number: index of the layer
    ///
    /// For instance: "1/*" return all children of the first layer, "../3" return the third layer of the parent
    /// of the current layer
    /// If the string starts with "/" then it's an aboslute path, otherwise it's a relative path.
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
