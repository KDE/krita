/*
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
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
#ifndef ITEMSTORE_H
#define ITEMSTORE_H

#include <QList>
#include <QRectF>

class KoShapeManager;
class KoShape;
class FolderShape;

class ItemStore {
public:
    ItemStore(KoShapeManager *shapeManager);
    ~ItemStore();

    void addFolder(FolderShape *folder);
    void removeFolder(FolderShape *folder);
    QList<FolderShape*> folders() const;
    void addShape(KoShape *shape);
    void removeShape(KoShape *shape);
    QList<KoShape*> shapes() const;

    FolderShape * mainFolder() const;

    QRectF loadShapeTypes();
    KoShapeManager *shapeManager() const { return m_shapeManager; }

private:
    KoShapeManager *m_shapeManager;
};

#endif
