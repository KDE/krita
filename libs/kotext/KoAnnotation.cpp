/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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

#include "KoAnnotation.h"

#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoTextInlineRdf.h>
#include <KoTextRangeManager.h>
#include <KoTextLoader.h>
#include <KoXmlNS.h>
#include <KoTextWriter.h>

#include "KoTextDocument.h"

#include <QTextDocument>
#include <QTextFrameFormat>
#include <QTextList>
#include <QTextBlock>
#include <QTextCursor>
#include <QWeakPointer>
#include <KDebug>

class KoAnnotation::Private
{
public:
    Private(const QTextDocument *doc)
        : document(doc),
          posInDocument(0) { }
    const QTextDocument *document;
    int posInDocument;
    QTextFrame *textFrame;

    // Name of this annotation. It is used to tie together the annotation and annotation-end tags
    QString name;

    // The actual contents of the annotation
    QString creator;
    QString date;
    QTextDocument contents;
};

KoAnnotation::KoAnnotation(const QTextCursor &cursor)
    : KoTextRange(cursor),
      d(new Private(cursor.block().document()))
{
}

KoAnnotation::~KoAnnotation()
{
    delete d;
}


void KoAnnotation::setName(const QString &name)
{
    d->name = name;
}

QString KoAnnotation::name() const
{
    return d->name;
}

QTextFrame *KoAnnotation::textFrame() const
{
    return d->textFrame;
}


void KoAnnotation::setMotherFrame(QTextFrame *frame)
{
    QTextCursor cursor(frame->lastCursorPosition());
    QTextFrameFormat format;
    format.setProperty(KoText::SubFrameType, KoText::NoteFrameType);
    d->textFrame = cursor.insertFrame(format);
    d->document = frame->document();
}

bool KoAnnotation::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    kDebug(32500) << "****** Start Load odf ******";
    KoTextLoader textLoader(context);
    QTextCursor cursor(d->textFrame);

    QString annotationName = element.attribute("name");

    const QString localName(element.localName());

    if (manager()) {
        // For cut and paste, make sure that the name is unique.
        d->name = createUniqueAnnotationName(manager()->annotationManager(), annotationName, false);

        if (localName == "annotation") {
            // We only support annotations for a point to start with.
            // point, not a region. If we encounter an annotation-end
            // tag, we will change that.
            setPositionOnlyMode(true);

            // Add inline Rdf to the annotation.
            if (element.hasAttributeNS(KoXmlNS::xhtml, "property") || element.hasAttribute("id")) {
                KoTextInlineRdf* inlineRdf = new KoTextInlineRdf(const_cast<QTextDocument*>(d->document), this);
                if (inlineRdf->loadOdf(element)) {
                    setInlineRdf(inlineRdf);
                }
                else {
                    delete inlineRdf;
                    inlineRdf = 0;
                }
            }

            // FIXME: Load more attributes here

            // Load the metadata (author, date) and contents here.
            KoXmlElement el;
            forEachElement(el, element) {
                if (el.localName() == "creator" && el.namespaceURI() == KoXmlNS::dc) {
                    d->creator = el.text();
                }
                else if (el.localName() == "date" && el.namespaceURI() == KoXmlNS::dc) {
                    d->date = el.text();
                }
                else if (el.localName() == "datestring" && el.namespaceURI() == KoXmlNS::meta) {
                    // FIXME: What to do here?
                }
          }
            textLoader.loadBody(element, cursor);

            kDebug(32500) << "****** End Load ******";
            kDebug(32500) << "loaded Annotation: " << d->creator << d->date;
        }
        else {
            // something pretty weird going on...
            return false;
        }
        return true;
    }
    return false;
}

void KoAnnotation::saveOdf(KoShapeSavingContext &context, int position, TagType tagType) const
{
    KoXmlWriter *writer = &context.xmlWriter();

    if ((tagType == StartTag) && (position == rangeStart())) {
        writer->startElement("office:annotation", false);
        writer->addAttribute("text:name", d->name.toUtf8());
        if (inlineRdf()) {
            inlineRdf()->saveOdf(context, writer);
        }

        writer->startElement("dc:creator", false);
        writer->addTextNode(d->creator);
        writer->endElement(); // dc:creator
        writer->startElement("dc:date", false);
        writer->addTextNode(d->date);
        writer->endElement(); // dc:date

        KoTextWriter textWriter(context);
        textWriter.write(d->document, d->textFrame->firstPosition(),d->textFrame->lastPosition());

        writer->endElement(); //office:annotation
    } else if ((tagType == EndTag) && (position == rangeEnd())) {
        writer->startElement("text:annotation-end", false);
        writer->addAttribute("text:name", d->name.toUtf8());
        writer->endElement();
    }
    // else nothing
}

QString KoAnnotation::createUniqueAnnotationName(const KoAnnotationManager* kam,
                                                 QString annotationName, bool isEndMarker)
{
    QString ret = annotationName;
    int uniqID = 0;

    while (true) {
        if (kam->annotation(ret)) {
            ret = QString("%1_%2").arg(annotationName).arg(++uniqID);
        } else {
            if (isEndMarker) {
                --uniqID;
                if (!uniqID)
                    ret = annotationName;
                else
                    ret = QString("%1_%2").arg(annotationName).arg(uniqID);
            }
            break;
        }
    }
    return ret;
}

QString KoAnnotation::creator() const
{
    // FIXME: I don't know but it was the result crash
    //return d->creator;
    return "creator";
}

QString KoAnnotation::date() const
{
    // FIXME: I don't know but it was the result crash
    //return d->date;
    return "data";
}

