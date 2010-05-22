/*
 * Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
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
#ifndef FOLDERSHAPEMODEL_H
#define FOLDERSHAPEMODEL_H

#include <KoShapeContainerModel.h>

class FolderShape;

class FolderShapeModel : public KoShapeContainerModel
{
public:
    FolderShapeModel(FolderShape *parent);

    /// add a content item to the folder
    virtual void add(KoShape *child);
    /// remove a content item from the folder
    virtual void remove(KoShape *child);
    /// ignored.
    virtual void setClipped(const KoShape *child, bool clipping);
    /// always returns true clipping any child to the folder outline
    virtual bool isClipped(const KoShape *child) const;
    virtual bool isChildLocked(const KoShape *child) const;
    /// always returns true, but thats irrelevant since folders don't rotate.
    virtual bool inheritsTransform(const KoShape *shape) const;
    /// ignored
    virtual void setInheritsTransform(const KoShape *shape, bool inherit);
    /// return content item count
    virtual int count() const;
    /// returns content items added earlier
    virtual QList<KoShape *> shapes() const;
    /// reimplemented from KoShapeContainerModel
    virtual void containerChanged(KoShapeContainer *container, KoShape::ChangeType type);
    /// reimplemented from KoShapeContainerModel
    virtual void childChanged(KoShape *child, KoShape::ChangeType type);

    /// called by the folder shape to allow us to reorganize the items in the folder
    void folderResized();

private:
    QList<KoShape*> m_icons;
    FolderShape *m_parent;
};

#endif
