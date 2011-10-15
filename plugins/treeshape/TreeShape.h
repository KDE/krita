/* This file is part of the KDE project

   Copyright (c) 2010 Cyril Oblikov <munknex@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef TREESHAPE_H
#define TREESHAPE_H

#include "KoShapeContainer.h"
#include "KoDocumentResourceManager.h"
#include <KoConnectionShape.h>

#define TREESHAPEID "TreeShape"

class KoViewConverter;
class KoShapeSavingContext;
class KoShapeLoadingContext;
class KoShape;
class KoConnectionShape;
class TreeLayout;
class QPainter;

class TreeShape : public KoShapeContainer
{
public:

    enum TreeType {
        OrgDown, OrgUp, OrgLeft, OrgRight,
        TreeLeft, TreeRight,
        MapClockwise, MapAntiClockwise,
        FollowParent
    };

    // ConnectionType is defined in KoConnectionShape

    enum RootType {
        Rectangle, Ellipse,
        None
    };


    TreeShape(KoDocumentResourceManager *documentResources=0);
    TreeShape(KoShape *root, KoDocumentResourceManager *documentResources=0);
    virtual ~TreeShape();
    virtual void setZIndex(int zIndex);
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintcontext);
    virtual bool hitTest(const QPointF &position) const;
    virtual void saveOdf(KoShapeSavingContext &context) const;
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    virtual void addChild(KoShape *tree, KoShape *connector);
    virtual KoShape* connector(KoShape *shape);
    virtual void setRoot(KoShape *shape, RootType type);
    virtual KoShape* root() const;
    virtual RootType rootType() const;
    virtual void setStructure(TreeShape::TreeType structure);
    virtual TreeShape::TreeType structure() const;
    virtual void setConnectionType(KoConnectionShape::Type type);
    virtual KoConnectionShape::Type connectionType() const;
    virtual QList<KoShape*> addNewChild();
    virtual void setNextShape(KoShape *shape);
    virtual KoShape* nextShape();
    virtual KoShape* proposePosition(KoShape* shape);
    virtual TreeType proposeStructure();

private:
//     virtual void shapeChanged(ChangeType type, KoShape *shape = 0);
    virtual TreeLayout *layout() const;

    KoDocumentResourceManager *m_documentResources;
    KoShape *m_nextShape;
};

#endif // KOTREESHAPE_H
