/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2012 C. Boemann <cbo@boemann.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoTextInlineRdf.h"
// lib
#include <opendocument/KoTextSharedSavingData.h>
#include <styles/KoCharacterStyle.h>
#include <KoBookmark.h>
#include <KoAnnotation.h>
#include <KoTextMeta.h>
#include <KoTextEditor.h>
#include <KoTextDocument.h>
// komain
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoElementReference.h>
// KDE
#include "TextDebug.h"
// Qt
#include <QTextCursor>
#include <QTextDocument>
#include <QTextTableCell>


#ifdef SHOULD_BUILD_RDF
#include <Soprano/Soprano>
enum Type {
    EmptyNode = Soprano::Node::EmptyNode,
    ResourceNode = Soprano::Node::ResourceNode,
    LiteralNode = Soprano::Node::LiteralNode,
    BlankNode = Soprano::Node::BlankNode
};
#else
enum Type {
    EmptyNode,
    ResourceNode,
    LiteralNode,
    BlankNode
};
#endif

class Q_DECL_HIDDEN KoTextInlineRdf::Private
{
public:
    Private(const QTextDocument *doc, const QTextBlock &b)
        : block(b)
        , document(doc)
    {
        isObjectAttributeUsed = false;
        sopranoObjectType = LiteralNode;
    }

    Private(const QTextDocument *doc, KoBookmark *b)
        : document(doc)
        , bookmark(b)
    {
        isObjectAttributeUsed = false;
        sopranoObjectType = LiteralNode;
    }

    Private(const QTextDocument *doc, KoAnnotation *b)
        : document(doc)
        , annotation(b)
    {
        isObjectAttributeUsed = false;
        sopranoObjectType = LiteralNode;
    }

    Private(const QTextDocument *doc, KoTextMeta *b)
        : document(doc)
        , kotextmeta(b)
    {
        isObjectAttributeUsed = false;
        sopranoObjectType = LiteralNode;
    }

    Private(const QTextDocument *doc, const QTextTableCell &c)
        : document(doc)
        , cell(c)
    {
        isObjectAttributeUsed = false;
        sopranoObjectType = LiteralNode;
    }

    Private(const QTextDocument *doc, KoSection *s)
        : document(doc)
        , section(s)
    {
        isObjectAttributeUsed = false;
        sopranoObjectType = LiteralNode;
    }


    QString id; // original xml:id

    //FIXME: design like this seems inapropriate, maybe
    // making Interface from KoTextInlineRdf will be better.
    // Just my thoughts.

    // where we might get the object value from
    QTextBlock block;

    // or document and one of bookmark, annotation, kotextmeta, ...
    QWeakPointer<const QTextDocument> document;
    QWeakPointer<KoBookmark> bookmark;
    QWeakPointer<KoAnnotation> annotation;
    QWeakPointer<KoTextMeta> kotextmeta;
    KoSection *section;
    QTextTableCell cell;

    QString subject;
    QString predicate;
    int sopranoObjectType;
    QString dt;

    // if the content="" attribute was used,
    // then isObjectAttributeUsed=1 and object=content attribute value.
    QString object;
    bool isObjectAttributeUsed;
};

KoTextInlineRdf::KoTextInlineRdf(const QTextDocument *doc, const QTextBlock &b)
    : QObject(const_cast<QTextDocument*>(doc))
    , d(new Private(doc, b))
{
}

KoTextInlineRdf::KoTextInlineRdf(const QTextDocument *doc, KoBookmark *b)
    : QObject(const_cast<QTextDocument*>(doc))
    , d(new Private(doc, b))
{
}

KoTextInlineRdf::KoTextInlineRdf(const QTextDocument *doc, KoAnnotation *b)
    : QObject(const_cast<QTextDocument*>(doc))
    , d(new Private(doc, b))
{
}

KoTextInlineRdf::KoTextInlineRdf(const QTextDocument *doc, KoTextMeta *b)
    : QObject(const_cast<QTextDocument*>(doc))
    , d(new Private(doc, b))
{
}

KoTextInlineRdf::KoTextInlineRdf(const QTextDocument *doc, const QTextTableCell &b)
    : QObject(const_cast<QTextDocument*>(doc))
    , d(new Private(doc, b))
{
}

KoTextInlineRdf::KoTextInlineRdf(const QTextDocument *doc, KoSection *s)
    : QObject(const_cast<QTextDocument*>(doc))
    , d(new Private(doc, s))
{
}

KoTextInlineRdf::~KoTextInlineRdf()
{
    debugText << " this:" << (void*)this;
    delete d;
}

bool KoTextInlineRdf::loadOdf(const KoXmlElement &e)
{
    d->id = e.attribute("id", QString());
    d->subject = e.attributeNS(KoXmlNS::xhtml, "about");
    d->predicate = e.attributeNS(KoXmlNS::xhtml, "property");
    d->dt = e.attributeNS(KoXmlNS::xhtml, "datatype");
    QString content = e.attributeNS(KoXmlNS::xhtml, "content");
    //
    // Content / triple object explicitly set through an attribute
    //
    if (e.hasAttributeNS(KoXmlNS::xhtml, "content")) {
        d->isObjectAttributeUsed = true;
        d->object = content;
    }
    return true;
}

bool KoTextInlineRdf::saveOdf(KoShapeSavingContext &context, KoXmlWriter *writer, KoElementReference id) const
{
    debugText << " this:" << (void*)this << " xmlid:" << d->id << "passed id" << id.toString();
    QString oldID = d->id;

    if (!id.isValid()) {
        id = KoElementReference();
    }

    QString newID = id.toString();
    if (KoTextSharedSavingData *sharedData =
            dynamic_cast<KoTextSharedSavingData *>(context.sharedData(KOTEXT_SHARED_SAVING_ID))) {
        sharedData->addRdfIdMapping(oldID, newID);
    }
    debugText << "oldID:" << oldID << " newID:" << newID;
    writer->addAttribute("xml:id", newID);
    if (!d->subject.isEmpty()) {
        writer->addAttribute("xhtml:about", d->subject);
    }
    if (!d->predicate.isEmpty()) {
        writer->addAttribute("xhtml:property", d->predicate);
    }
    if (!d->dt.isEmpty()) {
        writer->addAttribute("xhtml:datatype", d->dt);
    }
    if (d->isObjectAttributeUsed) {
        writer->addAttribute("xhtml:content", d->object);
    }
    debugText << "done..";
    return true;
}

QString KoTextInlineRdf::createXmlId()
{
    KoElementReference ref;
    return ref.toString();
}

QString KoTextInlineRdf::subject() const
{
    return d->subject;
}

QString KoTextInlineRdf::predicate() const
{
    return d->predicate;
}

QPair<int, int>  KoTextInlineRdf::findExtent() const
{
    if (d->bookmark && d->document) {
        return QPair<int, int>(d->bookmark.data()->rangeStart(), d->bookmark.data()->rangeEnd());
    }
    if (d->annotation && d->document) {
        return QPair<int, int>(d->annotation.data()->rangeStart(), d->annotation.data()->rangeEnd());
    }
    // FIXME: We probably have to do something with endAnnotation()
    //        too, but I don't know exactly what...
    if (d->kotextmeta && d->document) {
        KoTextMeta *e = d->kotextmeta.data()->endBookmark();
        if (!e) {
            return QPair<int, int>(0, 0);
        }
        // debugText << "(Semantic)meta... start:" << d->kotextmeta.data()->position() << " end:" << e->position();
        return QPair<int, int>(d->kotextmeta.data()->position(), e->position());
    }
    if (d->cell.isValid() && d->document) {
        QTextCursor b = d->cell.firstCursorPosition();
        QTextCursor e = d->cell.lastCursorPosition();
        return QPair<int, int>(b.position(), e.position());
    }

    if (d->section) {
        return d->section->bounds();
    }

    return QPair<int, int>(0, 0);
}

QString KoTextInlineRdf::object() const
{
    if (d->isObjectAttributeUsed) {
        return d->object;
    }

    KoTextDocument textDocument(d->document.data());

    if (d->bookmark && d->document) {
        QString ret  = d->bookmark.data()->text();
        return ret.remove(QChar::ObjectReplacementCharacter);
    }
    else if (d->kotextmeta && d->document) {
        // FIXME: Need to do something with endAnnotation?
        KoTextMeta *e = d->kotextmeta.data()->endBookmark();
        if (!e) {
            debugText << "Broken KoTextMeta, no end tag found!";
            return QString();
        } else {
            KoTextEditor *editor = textDocument.textEditor();
            editor->setPosition(d->kotextmeta.data()->position(), QTextCursor::MoveAnchor);
            editor->setPosition(e->position(), QTextCursor::KeepAnchor);
            QString ret = editor->selectedText();
            return ret.remove(QChar::ObjectReplacementCharacter);
        }
    }
    else if (d->cell.isValid() && d->document) {
        QTextCursor b = d->cell.firstCursorPosition();
        b.setPosition(d->cell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
        QString ret = b.selectedText();
        return ret.remove(QChar::ObjectReplacementCharacter);
    }

    return d->block.text();
}

int KoTextInlineRdf::sopranoObjectType() const
{
    return d->sopranoObjectType;
}

QString KoTextInlineRdf::xmlId() const
{
    return d->id;
}

void KoTextInlineRdf::setXmlId(const QString &id)
{
    d->id = id;
}

KoTextInlineRdf *KoTextInlineRdf::tryToGetInlineRdf(const QTextFormat &tf)
{
    if (!tf.hasProperty(KoCharacterStyle::InlineRdf)) {
        return 0;
    }
    QVariant v = tf.property(KoCharacterStyle::InlineRdf);
    return v.value<KoTextInlineRdf *>();
}

KoTextInlineRdf *KoTextInlineRdf::tryToGetInlineRdf(QTextCursor &cursor)
{
    QTextCharFormat cf = cursor.charFormat();
    if (!cf.hasProperty(KoCharacterStyle::InlineRdf)) {
        return 0;
    }
    QVariant v = cf.property(KoCharacterStyle::InlineRdf);
    return v.value<KoTextInlineRdf *>();
}

KoTextInlineRdf *KoTextInlineRdf::tryToGetInlineRdf(KoTextEditor *handler)
{
    QTextCharFormat cf = handler->charFormat();
    if (!cf.hasProperty(KoCharacterStyle::InlineRdf)) {
        return 0;
    }
    QVariant v = cf.property(KoCharacterStyle::InlineRdf);
    return v.value<KoTextInlineRdf *>();
}

void KoTextInlineRdf::attach(KoTextInlineRdf *inlineRdf, QTextCursor &cursor)
{
    QTextCharFormat format = cursor.charFormat();
    QVariant v = QVariant::fromValue(inlineRdf);
    format.setProperty(KoCharacterStyle::InlineRdf, v);
    cursor.mergeCharFormat(format);
}
