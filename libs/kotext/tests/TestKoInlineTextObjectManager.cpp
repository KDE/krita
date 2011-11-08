/* This file is part of the KDE project
 *
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
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
#include "TestKoInlineTextObjectManager.h"

#include <QtTest/QTest>
#include <QDebug>
#include <QString>
#include <QTextDocument>
#include <QList>
#include <QTextCursor>
#include <QTextCharFormat>

#include <KoInlineObject.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextEditor.h>
#include <KoTextDocument.h>
#include <KoBookmark.h>

class DummyInlineObject : public KoInlineObject
{
public:

    DummyInlineObject(bool propertyListener)
        : KoInlineObject(propertyListener)
        , m_position(-1)
    {
    }

    virtual ~DummyInlineObject() {}

    virtual void saveOdf(KoShapeSavingContext &/*context*/)
    {
        // dummy impl
    }

    virtual bool loadOdf(const KoXmlElement&, KoShapeLoadingContext&)
    {
        // dummy impl
        return false;
    }

    virtual void updatePosition(const QTextDocument *document, int posInDocument, const QTextCharFormat &/*format*/)
    {
        Q_ASSERT(posInDocument <= document->toPlainText().size());
        m_position = posInDocument;
    }

    virtual void resize(const QTextDocument */*document*/, QTextInlineObject /*object*/,
                        int /*posInDocument*/, const QTextCharFormat &/*format*/, QPaintDevice */*pd*/)
    {
        // dummy impl
    }

    virtual void paint(QPainter &/*painter*/, QPaintDevice */*pd*/, const QTextDocument */*document*/,
                       const QRectF &/*rect*/, QTextInlineObject /*object*/, int /*posInDocument*/, const QTextCharFormat &/*format*/)
    {
        // dummy impl
    }

    virtual void propertyChanged(Property /*property*/, const QVariant &value)
    {
        m_property = value;
    }

    QVariant m_property;
    int m_position;

};

const QString lorem(
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor"
    "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud"
    "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla"
    "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia"
    "deserunt mollit anim id est laborum.\n"
    );




void TestKoInlineTextObjectManager::testCreation()
{
    KoInlineTextObjectManager *manager = new KoInlineTextObjectManager();
    Q_ASSERT(manager);
    Q_ASSERT(manager->bookmarkManager());
    Q_ASSERT(manager->variableManager());
    delete manager;
}

void TestKoInlineTextObjectManager::testInsertInlineObject()
{
    QTextDocument doc;
    KoInlineTextObjectManager manager;
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&manager);

    KoTextEditor editor(&doc);
    editor.insertText(lorem);

    DummyInlineObject *obj = new DummyInlineObject(false);
    Q_ASSERT(obj->id() == -1);
    Q_ASSERT(obj->manager() == 0);
    manager.insertInlineObject(*editor.cursor(), obj);
    Q_ASSERT(obj->id() == 1);
    Q_ASSERT(obj->manager() == &manager);

    // we should have a replacement char associated with this item
    QTextCursor cursor = doc.find(QString(QChar::ObjectReplacementCharacter), 0);
    Q_ASSERT(cursor.position() != 0);

    // the associated character format should be the same as the one of the object
    QTextCharFormat fmt = cursor.charFormat();
    Q_ASSERT(fmt.property(KoInlineTextObjectManager::InlineInstanceId).toInt() == obj->id());

    // before updatePosition() is called, the object doesn't know the position of the
    // replacement character
    Q_ASSERT(obj->m_position == -1);

    // call updateposition -- it is the position of the replacement character.
    obj->updatePosition(&doc, cursor.position(), fmt);

    Q_ASSERT(obj->m_position == cursor.position());
}

void TestKoInlineTextObjectManager::testRetrieveInlineObject()
{
    QTextDocument doc;
    KoInlineTextObjectManager manager;
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&manager);

    KoTextEditor editor(&doc);
    editor.insertText(lorem);

    DummyInlineObject *obj = new DummyInlineObject(false);
    Q_ASSERT(obj->id() == -1);
    Q_ASSERT(obj->manager() == 0);
    manager.insertInlineObject(*editor.cursor(), obj);
    Q_ASSERT(obj->id() == 1);
    Q_ASSERT(obj->manager() == &manager);

    manager.insertInlineObject(*editor.cursor(), new DummyInlineObject(false));
    editor.insertText(lorem);
    manager.insertInlineObject(*editor.cursor(), new DummyInlineObject(false));
    editor.insertText(lorem);
    manager.insertInlineObject(*editor.cursor(), new DummyInlineObject(false));
    editor.insertText(lorem);

    // by id
    KoInlineObject *obj2 = manager.inlineTextObject(1);
    Q_ASSERT(obj2 == obj);
    Q_UNUSED(obj2) // not really unused, but gcc thinks so.

    // by cursor
    editor.setPosition(444);
    obj2 = manager.inlineTextObject(*editor.cursor());
    Q_ASSERT(obj2 == obj);

    // by charformat
    // we should have a replacement char associated with this item
    QTextCursor cursor = doc.find(QString(QChar::ObjectReplacementCharacter), 0);
    QTextCharFormat fmt = cursor.charFormat();
    obj2 = manager.inlineTextObject(fmt);
    Q_ASSERT(obj2 == obj);

}

void TestKoInlineTextObjectManager::testRemoveInlineObject()
{
    QTextDocument doc;
    KoInlineTextObjectManager manager;
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&manager);
    KoTextEditor editor(&doc);
    DummyInlineObject *obj = new DummyInlineObject(true);
    manager.insertInlineObject(*editor.cursor(), obj);

    int id = obj->id();

    QTextCursor cursor = doc.find(QString(QChar::ObjectReplacementCharacter), 0);
    Q_ASSERT(cursor.position() == 1);

    manager.removeInlineObject(cursor);

    KoInlineObject *obj2 = manager.inlineTextObject(id);
    Q_ASSERT(obj2 == 0);

    // we cannot find the inline char anymore
    cursor = doc.find(QString(QChar::ObjectReplacementCharacter), 0);
    Q_ASSERT(cursor.position() == -1);

    // this should not crash, even though we were a listener
    manager.setProperty(KoInlineObject::User, "bla");

    // now insert a bookmark and remove it. It should also be gone from the bookmark manager
    KoBookmark *bm = new KoBookmark(&doc);
    bm->setType(KoBookmark::SinglePosition);
    bm->setName("single!");
    manager.insertInlineObject(*editor.cursor(), bm);
    Q_ASSERT(manager.bookmarkManager()->bookmarkNameList().contains("single!"));
    manager.removeInlineObject(bm);
    Q_ASSERT(!manager.bookmarkManager()->bookmarkNameList().contains("single!"));
    Q_ASSERT(!manager.bookmarkManager()->retrieveBookmark("single!"));

}

void TestKoInlineTextObjectManager::testListenToProperties()
{
    QTextDocument doc;
    KoInlineTextObjectManager manager;
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&manager);
    KoTextEditor editor(&doc);
    DummyInlineObject *obj = new DummyInlineObject(true);
    manager.insertInlineObject(*editor.cursor(), obj);
    manager.setProperty(KoInlineObject::User, "bla");
    Q_ASSERT(obj->m_property.toString() == "bla");

}


QTEST_MAIN(TestKoInlineTextObjectManager)

#include "TestKoInlineTextObjectManager.moc"
