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



#ifndef TEST_KO_INLINE_TEXT_OBJECT_MANAGER_H
#define TEST_KO_INLINE_TEXT_OBJECT_MANAGER_H

#include <QObject>
#include <QTest>
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
        Q_ASSERT(posInDocument <= document->toPlainText().size()); Q_UNUSED(document);
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

class TestKoInlineTextObjectManager : public QObject
{
    Q_OBJECT

private slots:

    void testCreation();
    void testInsertInlineObject();
    void testRetrieveInlineObject();
    void testRemoveInlineObject();
    void testListenToProperties();

};

#endif // TEST_KO_INLINE_TEXT_OBJECT_MANAGER_H
