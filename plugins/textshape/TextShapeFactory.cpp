/* This file is part of the KDE project
 * Copyright (C) 2006-2007,2009,2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#include "TextShapeFactory.h"
#include "TextShape.h"

#include <KoProperties.h>
#include <KoShape.h>
#include <KoTextDocument.h>
#include <KoTextShapeData.h>
#include <KoXmlNS.h>
#include <KoStyleManager.h>
#include <KoResourceManager.h>
#include <KoInlineTextObjectManager.h>
#include <changetracker/KoChangeTracker.h>
#include <KoImageCollection.h>

#include <klocale.h>
#include <KUndoStack>
#include <QTextCursor>

TextShapeFactory::TextShapeFactory(QObject *parent)
        : KoShapeFactoryBase(parent, TextShape_SHAPEID, i18n("Text"))
{
    setToolTip(i18n("A shape that shows text"));
    QList<QPair<QString, QStringList> > odfElements;
    odfElements.append(QPair<QString, QStringList>(KoXmlNS::draw, QStringList("text-box")));
    odfElements.append(QPair<QString, QStringList>(KoXmlNS::table, QStringList("table")));
    setOdfElements(odfElements);
    setLoadingPriority(1);

    KoShapeTemplate t;
    t.name = i18n("Text");
    t.icon = "x-shape-text";
    t.toolTip = i18n("Text Shape");
    KoProperties *props = new KoProperties();
    t.properties = props;
    props->setProperty("demo", true);
    addTemplate(t);
}

KoShape *TextShapeFactory::createDefaultShape(KoResourceManager *documentResources) const
{
    KoInlineTextObjectManager *manager = 0;
    if (documentResources && documentResources->hasResource(KoText::InlineTextObjectManager)) {
        QVariant variant = documentResources->resource(KoText::InlineTextObjectManager);
        manager = variant.value<KoInlineTextObjectManager*>();
    }
    TextShape *text = new TextShape(manager);
    if (documentResources) {
        KoTextDocument document(text->textShapeData()->document());
        document.setUndoStack(documentResources->undoStack());

        if (documentResources->hasResource(KoText::StyleManager)) {
            KoStyleManager *styleManager = documentResources->resource(KoText::StyleManager).value<KoStyleManager*>();
            document.setStyleManager(styleManager);
        }
        if (documentResources->hasResource(KoText::PageProvider)) {
            KoPageProvider *pp = static_cast<KoPageProvider *>(documentResources->resource(KoText::PageProvider).value<void*>());
            text->setPageProvider(pp);
        }
        if (documentResources->hasResource(KoText::ChangeTracker)) {
            KoChangeTracker *changeTracker = documentResources->resource(KoText::ChangeTracker).value<KoChangeTracker*>();
            document.setChangeTracker(changeTracker);
        }

        text->setImageCollection(documentResources->imageCollection());
    }

    return text;
}

KoShape *TextShapeFactory::createShape(const KoProperties *params, KoResourceManager *documentResources) const
{
    TextShape *shape = static_cast<TextShape*>(createDefaultShape(documentResources));
    shape->textShapeData()->document()->setUndoRedoEnabled(false);
    shape->setSize(QSizeF(300, 200));
    shape->setDemoText(params->boolProperty("demo"));
    QString text("text");
    if (params->contains(text)) {
        KoTextShapeData *shapeData = qobject_cast<KoTextShapeData*>(shape->userData());
        QTextCursor cursor(shapeData->document());
        cursor.insertText(params->stringProperty(text));
    }
    if (documentResources) {
        shape->setImageCollection(documentResources->imageCollection());
    }
    shape->textShapeData()->document()->setUndoRedoEnabled(true);
    return shape;
}

bool TextShapeFactory::supports(const KoXmlElement & e) const
{
    return (e.localName() == "text-box" && e.namespaceURI() == KoXmlNS::draw) ||
        (e.localName() == "table" && e.namespaceURI() == KoXmlNS::table);
}

void TextShapeFactory::newDocumentResourceManager(KoResourceManager *manager)
{
    QVariant variant;
    variant.setValue<KoInlineTextObjectManager*>(new KoInlineTextObjectManager(manager));
    manager->setResource(KoText::InlineTextObjectManager, variant);

    if (!manager->hasResource(KoDocumentResource::UndoStack)) {
        kWarning(32500) << "No KUndoStack found in the document resource manager, creating a new one";
        manager->setUndoStack(new KUndoStack(manager));
    }
    variant.setValue(new KoStyleManager(manager));
    manager->setResource(KoText::StyleManager, variant);

    if (!manager->imageCollection())
        manager->setImageCollection(new KoImageCollection(manager));
}

#include <TextShapeFactory.moc>
