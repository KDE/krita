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
#include "ItemStore.h"

#include "ClipboardProxyShape.h"
#include "FolderShape.h"
#include "TemplateShape.h"
#include "GroupShape.h"
#include "FolderBorder.h"
#include "Canvas.h"

#include <KoShapeManager.h>
#include <KoProperties.h>
#include <KoShapeRegistry.h>
#include <KoOdfPaste.h>
#include <KoOdfReadStore.h>
#include <KoOdfReadStore.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoGlobal.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <KLocale>
#include <KConfigGroup>
#include <KGlobal>
#include <KDebug>

class DummyShapeController : public KoShapeControllerBase
{
public:
    DummyShapeController() {}

    virtual void addShape( KoShape* ) {}
    virtual void removeShape( KoShape* ) {}
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
    DummyShapeController shapeController;
};


Q_GLOBAL_STATIC(ItemStorePrivate, s_itemStorePrivate)

/// ItemStorePrivate
ItemStorePrivate::ItemStorePrivate()
    : mainFolder(0), currentClipboard(0)
{
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));
}

void ItemStorePrivate::addFolder(FolderShape *folder)
{
    Q_ASSERT(folder);
    if (folders.contains(folder))
        return;
    mainFolder = folder;
    if (!folders.isEmpty()) {
        folder->setBorder(folders[0]->border());
        int zIndex = 0;
        foreach (FolderShape *fs, folders)
            zIndex = qMax(zIndex, fs->zIndex());
        folder->setZIndex(zIndex+1);
    }
    folders.append(folder);
    if (folders.count() > 1)
        mainFolder = 0;
    foreach (KoShapeManager *sm, shapeManagers)
        sm->addShape(folder);
}

void ItemStorePrivate::removeFolder(FolderShape *folder)
{
    Q_ASSERT(folder);
    Q_ASSERT(folder != mainFolder); // can't remove the last folder
    foreach (KoShapeManager *sm, shapeManagers)
        sm->remove(folder);
    folders.removeAll(folder);
    if (folders.count() == 1)
        mainFolder = folders[0];
}

void ItemStorePrivate::addShape(KoShape *shape)
{
    if (shapes.contains(shape))
        return;
    foreach (KoShapeManager *sm, shapeManagers)
        sm->addShape(shape);
    shapes.append(shape);
}

void ItemStorePrivate::removeShape(KoShape *shape)
{
    foreach (KoShapeManager *sm, shapeManagers)
        sm->remove(shape);
    shapes.removeAll(shape);
}

void ItemStorePrivate::addUser(KoShapeManager *sm)
{
    shapeManagers.append(sm);
}

void ItemStorePrivate::removeUser(KoShapeManager *sm)
{
    if (sm == 0)
        return;
    shapeManagers.removeAll(sm);
    // remove all shapes from this shape manager to avoid a crash later
    sm->setShapes(QList<KoShape *>());
    KConfigGroup conf = KoGlobal::kofficeConfig()->group("ShapeSelectorPlugin");
    const int previouslyConfiguredBooks = conf.readEntry("books", 0);
    conf.writeEntry("books", folders.size());
    int index = 1;
    foreach(FolderShape *folder, folders) {
        conf.writeEntry(QString::fromLatin1("book.%1_name").arg(index).toAscii().data(), folder->name());
        if (folder == mainFolder)
            continue;
        conf.writeEntry(QString::fromLatin1("book.%1_position").arg(index).toAscii().data(), folder->position());
        conf.writeEntry(QString::fromLatin1("book.%1_size").arg(index).toAscii().data(), folder->size());
        conf.writeEntry(QString::fromLatin1("book.%1_items").arg(index).toAscii().data(), folder->save().toByteArray());
        ++index;
    }
    for (int i=folders.size()+1; i <= previouslyConfiguredBooks; i++) { // remove surplus
        conf.deleteEntry(QString::fromLatin1("book.%1_name").arg(i).toAscii().data());
        conf.deleteEntry(QString::fromLatin1("book.%1_position").arg(i).toAscii().data());
        conf.deleteEntry(QString::fromLatin1("book.%1_size").arg(i).toAscii().data());
        conf.deleteEntry(QString::fromLatin1("book.%1_items").arg(i).toAscii().data());
    }

    if (shapeManagers.count() == 0) { // last one
        qDeleteAll(folders);
        folders.clear();
        qDeleteAll(shapes);
        shapes.clear();
    }
}

void ItemStorePrivate::clipboardChanged()
{
    const QMimeData *data = QApplication::clipboard()->mimeData(QClipboard::Clipboard);
    QByteArray bytes = data->data(OASIS_MIME);
    KoShape *shape = ItemStore::createShapeFromPaste(bytes);

    if (shape)
        setClipboardShape(new ClipboardProxyShape(shape, bytes));
}

void ItemStorePrivate::setClipboardShape(ClipboardProxyShape *shape)
{
    if (currentClipboard) {
        shape->setParent(currentClipboard->parent());
        shape->setPosition(currentClipboard->position());
        removeShape(currentClipboard);
        delete currentClipboard;
    } else {
        // find a good default spot for the new clipboard shape.
        if (mainFolder)
            shape->setParent(mainFolder);
        else  // TODO is there a better way to get it to be centered or something?
            shape->setAbsolutePosition( QPointF( 50, 50) );
    }
    currentClipboard = shape;
    addShape(currentClipboard);
}

/// ItemStore
ItemStore::ItemStore(ShapeSelector *parent)
{
    m_canvas = new Canvas(parent, this);
    m_shapeManager = new KoShapeManager(m_canvas);

    if (parent) {
        parent->setWidget(m_canvas);
        QObject::connect(m_canvas, SIGNAL(resized(const QSize&)), parent, SLOT(setSize(const QSize &)));
    }

    s_itemStorePrivate()->addUser(m_shapeManager);
    if (s_itemStorePrivate()->shapeManagers.count() > 1) {
        KoShapeManager *other = s_itemStorePrivate()->shapeManagers.first();
        m_shapeManager->setShapes(other->shapes());
    }
}

ItemStore::~ItemStore()
{
    s_itemStorePrivate()->removeUser(m_shapeManager);
    delete m_shapeManager;
}

void ItemStore::addFolder(FolderShape *folder)
{
    s_itemStorePrivate()->addFolder(folder);
}

void ItemStore::removeFolder(FolderShape *folder)
{
    s_itemStorePrivate()->removeFolder(folder);
}

QList<FolderShape*> ItemStore::folders() const
{
    return s_itemStorePrivate()->folders;
}

void ItemStore::addShape(KoShape *shape)
{
    s_itemStorePrivate()->addShape(shape);
}

void ItemStore::removeShape(KoShape *shape)
{
    s_itemStorePrivate()->removeShape(shape);
}

QList<KoShape*> ItemStore::shapes() const
{
    return s_itemStorePrivate()->shapes;
}

FolderShape * ItemStore::mainFolder() const
{
    return s_itemStorePrivate()->mainFolder;
}

QRectF ItemStore::loadShapeTypes()
{
    qreal maxHeight = 0;
    if (s_itemStorePrivate()->shapeManagers.count() > 1)
        return QRectF(); // someone else already did the init.

    FolderShape *mainFolder = 0;

    KConfigGroup conf = KoGlobal::kofficeConfig()->group("ShapeSelectorPlugin");

    QRectF boundingRect;
    const int books = conf.readEntry("books", 0);
    for (int i=1; i <= books; i++) {
        FolderShape *folder = new FolderShape();
        folder->setName(
            conf.readEntry(QString::fromLatin1("book.%1_name").arg(i).toAscii().data(), "Unnamed"));
        folder->setPosition(
            conf.readEntry(QString::fromLatin1("book.%1_position").arg(i).toAscii().data(), QPointF()));
        folder->setSize(
            conf.readEntry(QString::fromLatin1("book.%1_size").arg(i).toAscii().data(), QSizeF(100, 100)));
        QByteArray children = conf.readEntry(QString::fromLatin1("book.%1_items").arg(i).toAscii().data(), QByteArray());
        if (children.size()) {
            QDomDocument doc;
            QString error;
            int line, column;
            if (doc.setContent(children, false, &error, &line, &column)) {
                folder->load(doc);
                foreach(KoShape *child, folder->shapes())
                    s_itemStorePrivate()->addShape(child);
            } else {
                kWarning(31000) << "ERROR: Could not parse xml for folder" << i << "at Line" << line << "Column" << column;
                kWarning(31000) << "  " << error;
            }
        }
        s_itemStorePrivate()->addFolder(folder);
        if (mainFolder == 0) {
            mainFolder = folder;
            if (books > 1)
                mainFolder->setBorder(new FolderBorder());
            boundingRect = mainFolder->boundingRect();
        } else {
            boundingRect = boundingRect.unite(folder->boundingRect());
        }
    }

    if (mainFolder == 0) {
        mainFolder = new FolderShape();
        mainFolder->setName(i18n("Templates"));
        boundingRect = mainFolder->boundingRect();
        s_itemStorePrivate()->addFolder(mainFolder);
    }

    foreach (const QString &id, KoShapeRegistry::instance()->keys()) {
        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value(id);
        if (factory->hidden())
            continue;
        bool oneAdded=false;
        foreach (const KoShapeTemplate &shapeTemplate, factory->templates()) {
            oneAdded=true;
            TemplateShape *ts = new TemplateShape(shapeTemplate);
            KoShapeTemplate t1 = ts->shapeTemplate();
            foreach(KoShape *shape, s_itemStorePrivate()->shapes) {
                TemplateShape *t = dynamic_cast<TemplateShape*>(shape);
                if (t == 0)
                    continue;
                KoShapeTemplate t2 = t->shapeTemplate();
                if (t1.id == t2.id && t1.properties && t2.properties && t1.properties->operator==(*t2.properties)) { // already there
                    delete ts;
                    ts = 0;
                    break;
                }
            }
            if (ts) {
                s_itemStorePrivate()->addShape(ts);
                mainFolder->addShape(ts);
                maxHeight = qMax(maxHeight, ts->size().height());
            }
        }
        if (!oneAdded) {
            KoShape *group = new GroupShape(factory);
            mainFolder->addShape(group);
            s_itemStorePrivate()->addShape(group);
        }
    }

    return boundingRect;
}

ClipboardProxyShape * ItemStore::clipboardShape() const
{
    return s_itemStorePrivate()->currentClipboard;
}

void ItemStore::setClipboardShape(ClipboardProxyShape *shape)
{
    s_itemStorePrivate()->setClipboardShape(shape);
}


// static
KoShape *ItemStore::createShapeFromPaste(const QByteArray &bytes)
{
    class Paster : public KoOdfPaste {
      public:
        Paster(KoShapeControllerBase *controller)
            : m_shape(0), m_shapeController(controller)
        {
        }

        bool process(const KoXmlElement &body, KoOdfReadStore &odfStore)
        {
            KoOdfLoadingContext loadingContext(odfStore.styles(), odfStore.store());
            KoShapeLoadingContext context(loadingContext, m_shapeController->resourceManager());

            KoXmlElement element;
            forEachElement(element, body) {
                m_shape = KoShapeRegistry::instance()->createShapeFromOdf(element, context);
                if (m_shape)
                    return true;
            }
            return false;
        }

        KoShape *shape() { return m_shape; }

      private:
        KoShape *m_shape;
        KoShapeControllerBase *m_shapeController;
    };

    Paster paster(&s_itemStorePrivate()->shapeController);
    paster.paste(KoOdf::Text, bytes);
    return paster.shape();
}

KoShapeControllerBase *ItemStore::shapeController()
{
    return &s_itemStorePrivate()->shapeController;
}

#include <ItemStore.moc>
