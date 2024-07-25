/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOSVGTEXTLOADER_H
#define KOSVGTEXTLOADER_H

#include "KoSvgTextShape.h"
#include "kritaflake_export.h"


/// Loading an SVG text is somewhat intricate, so we use a KoSvgTextLoader to keep
/// track of where we are in the tree.
class KoSvgTextLoader {
public:
    KoSvgTextLoader(KoSvgTextShape *shape);
    ~KoSvgTextLoader();

    /// Set the current node to its first child, entering the subtree.
    void enterNodeSubtree();

    /// Set the current node to its parent, leaving the subtree.
    void leaveNodeSubtree();

    /// Switch to next node.
    void nextNode();

    /// Create a new text node.
    bool loadSvg(const QDomElement &element, SvgLoadingContext &context, bool root = false);
    /// Loads the textt into the current node.
    bool loadSvgText(const QDomText &text, SvgLoadingContext &context);

    /// Set the style info from the shape. This is necessary because SVGParser only understands loading the basic style into a KoShape.
    void setStyleInfo(KoShape* s);
    /// Set the textPath on the current node.
    void setTextPathOnCurrentNode(KoShape *s);
private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // KOSVGTEXTLOADER_H
