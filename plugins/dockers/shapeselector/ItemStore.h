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
#include <QObject>

class KoShapeManager;
class KoShape;
class FolderShape;
class ClipboardProxyShape;

class ItemStore
{
public:
    ItemStore();
    explicit ItemStore(KoShapeManager *shapeManager);
    ~ItemStore();

    void addFolder(FolderShape *folder);
    void removeFolder(FolderShape *folder);
    QList<FolderShape*> folders() const;
    void addShape(KoShape *shape);
    void removeShape(KoShape *shape);
    QList<KoShape*> shapes() const;

    FolderShape * mainFolder() const;
    ClipboardProxyShape * clipboardShape() const;
    void setClipboardShape(ClipboardProxyShape *shape);

    QRectF loadShapeTypes();
    KoShapeManager *shapeManager() const { return m_shapeManager; }

    static KoShape *createShapeFromPaste(QByteArray &bytes);

private:
    KoShapeManager *m_shapeManager;
};

class ItemStorePrivate : public QObject
{
    Q_OBJECT
public:
    ItemStorePrivate();
    void addFolder(FolderShape *folder);
    void removeFolder(FolderShape *folder);
    void addShape(KoShape *shape);
    void removeShape(KoShape *shape);
    void addUser(KoShapeManager *sm);
    void removeUser(KoShapeManager *sm);
    void setClipboardShape(ClipboardProxyShape *shape);

private slots:
    void clipboardChanged();

public:
    QList<KoShape*> shapes;
    QList<FolderShape *> folders;
    QList<KoShapeManager*> shapeManagers;
    FolderShape *mainFolder;
    ClipboardProxyShape *currentClipboard;
};

#endif
