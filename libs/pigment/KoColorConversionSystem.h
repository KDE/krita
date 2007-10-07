/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef _KO_COLOR_CONVERSION_SYSTEM_H_
#define _KO_COLOR_CONVERSION_SYSTEM_H_

class KoColorSpace;
class KoColorSpaceFactory;
#include "KoColorConversionTransformation.h"

#include <QList>

#include <pigment_export.h>

/**
 * This class hold the logic related to pigment's Color Conversion System. It's
 * basically a graph containing all the possible color transformation between
 * the color spaces. The most usefull functions are createColorConverter to create
 * a color conversion between two color spaces, and insertColorSpace which is called
 * by KoColorSpaceRegistry each time a new color space is added to the registry.
 */
class PIGMENT_EXPORT KoColorConversionSystem {
        struct Node;
        struct Vertex;
        struct NodeKey;
        struct Path;
        friend uint qHash(const KoColorConversionSystem::NodeKey &key);
    public:
        /**
         * Construct a Color Conversion System, leave to the KoColorSpaceRegistry to
         * create it.
         */
        KoColorConversionSystem();
        ~KoColorConversionSystem();
        /**
         * This function is called by the KoColorSpaceRegistry to add a new color space
         * to the graph of transformation.
         */
        void insertColorSpace(const KoColorSpaceFactory*);
        /**
         * This function is called by the color space to create a color conversion
         * between two color space. This function search in the graph of transformations
         * the best possible path between the two color space.
         */
        KoColorConversionTransformation* createColorConverter(const KoColorSpace * srcColorSpace, const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const;
    public:
        /**
         * This function return a text that can be compiled using dot to display
         * the graph of color conversion connection.
         */
        QString toDot() const;
        /**
         * This function return a text that can be compiled using dot to display
         * the graph of color conversion connection, with a red link to show the
         * path of the best color conversion.
         */
        QString bestPathToDot(QString srcModelId, QString srcDepthId, QString dstModelId, QString dstDepthId) const;
    public:
        /**
         * @return true if there is a path between two color spaces
         */
        bool existsPath( QString srcModelId, QString srcDepthId, QString dstModelId, QString dstDepthId ) const;
    private:
        QString vertexToDot(Vertex* v, QString options) const;
    private:
        /**
         * Query the registry to get the color space associated with this
         * node. (default profile)
         */
        KoColorSpace* defaultColorSpaceForNode(const Node* node) const;
        /**
         * @return the node corresponding to that key, or create it if needed
         */
        Node* nodeFor(const NodeKey& key);
        const Node* nodeFor(const NodeKey& key) const;
        /**
         * @return the node associated with that key, and create it if needed
         */
        Node* nodeFor(QString colorModelId, QString colorDepthId);
        const Node* nodeFor(QString colorModelId, QString colorDepthId) const;
        /**
         * @return the vertex between two nodes, or null if the vertex doesn't exist
         */
        Vertex* vertexBetween(Node* srcNode, Node* dstNode);
        /**
         * create a vertex between two nodes and return it.
         */
        Vertex* createVertex(Node* srcNode, Node* dstNode);
        /**
         * looks for the best path between two nodes
         */
        Path* findBestPath(const Node* srcNode, const Node* dstNode) const;
        /**
         * Delete all the pathes of the list given in argument.
         */
        void deletePathes(QList<KoColorConversionSystem::Path*> pathes) const;
        /**
         * Don't call that function, but raher findBestPath
         * @internal
         */
        template<bool ignoreHdr, bool ignoreColorCorrectness>
        inline Path* findBestPathImpl(const Node* srcNode, const Node* dstNode) const;
        /**
         * Don't call that function, but raher findBestPath
         * @internal
         */
        template<bool ignoreHdr>
        inline Path* findBestPathImpl(const Node* srcNode, const Node* dstNode) const;
    private:
        struct Private;
        Private* const d;
};

#endif
