/* This file is part of the KDE project
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef TREE_H
#define TREE_H

#include "KoShapeContainer.h"

#include "flake_export.h"

#define TREEID "Tree"

class KoViewConverter;
class KoShapeSavingContext;
class KoShapeLoadingContext;
class KoShape;
class KoConnectionShape;
class Layout;
class QPainter;

class FLAKE_EXPORT Tree : public KoShapeContainer
{

public:
    Tree();
    Tree(KoShape *root);
    virtual ~Tree();
    virtual QList<KoShape*> addNewChild();
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter);
    virtual bool hitTest(const QPointF &position) const;
    virtual void saveOdf(KoShapeSavingContext &context) const;
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

private:
//     virtual void shapeChanged(ChangeType type, KoShape *shape = 0);
    virtual Layout *layout() const;

    uint m_structure;
};

#endif // KOTREE_H
