/*
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

    virtual void add(KoShape *child);
    virtual void remove(KoShape *child);
    virtual void setClipped(const KoShape *child, bool clipping);
    virtual bool isClipped(const KoShape *child) const;
    virtual bool isChildLocked(const KoShape *child) const;
    virtual int count() const;
    virtual QList<KoShape *> shapes() const;
    virtual void containerChanged(KoShapeContainer *container);
    virtual void childChanged(KoShape *child, KoShape::ChangeType type);

    void folderResized();

private:
    QList<KoShape*> m_icons;
    FolderShape *m_parent;
};

#endif
