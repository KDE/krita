/*
 * Copyright (C) 2008-2010 Thomas Zander <zander@kde.org>
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

/**
 * This is the data storage for a shape selector.
 * For every Canvas there is an ItemStore, the itemStore is holding all
 * the data that is shown on the Canvas.
 * This class will load all information from the user preferences and
 * find all the KoShapeFactoryBase objects on the users system to display
 * in folders.
 *
 * The ItemStore has two modes in which it operates, it either has exactly
 * one folder or it has many different folders. The mainFolder() getter shows
 * the different modes.
 *
 * Note that the ItemStore has a global static back-end called the ItemStorePrivate
 * this is used to transparently share all the data so that adding a shape in one
 * view will automatically have this change also shown in a second koffice document
 * view.
 */
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

    /**
     * Return the mainFolder if there is one.
     * The shape selector either has one full-screen main folder, or any number of
     * folders which can be shaped and positioned anywhere the user left them.
     * This method will return null (0) in case there are many folders and thus not
     * one specific main folder.
     * Note that if there is a mainFolder the user can not drag or resize the folder.
     */
    FolderShape *mainFolder() const;
    ClipboardProxyShape *clipboardShape() const;
    void setClipboardShape(ClipboardProxyShape *shape);

    QRectF loadShapeTypes();
    KoShapeManager *shapeManager() const { return m_shapeManager; }

    static KoShape *createShapeFromPaste(QByteArray &bytes);

private:
    KoShapeManager *m_shapeManager;
};

/**
 * This class holds the actual data that the ItemStore provides getters for.
 * The ItemStorePrivate is referenced via a global static and thus there is
 * at most one instance in memory at any time. (singleton pattern)
 * When there is more than one shape selector docker present in a process
 * they will implicitly share the ItemStorePrivate instance and thus any
 * changes in folders or even in the clipboard shape made will only be done
 * exactly one time for all the dockers. This has the immediate advantage that
 * adding or removing a folder will be synchorized accross all docker instances.
 */
class ItemStorePrivate : public QObject
{
    Q_OBJECT
public:
    ItemStorePrivate();
    void addFolder(FolderShape *folder);
    void removeFolder(FolderShape *folder);
    void addShape(KoShape *shape);
    void removeShape(KoShape *shape);
    /// register a KoShapeManager as a user of this store so repaints can be made.
    void addUser(KoShapeManager *sm);
    /// remove a KoShapeManager as a user of this store to no longer report repaints to it.
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
