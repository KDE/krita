/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>
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
#ifndef KOINLINECITE_H
#define KOINLINECITE_H

#include "KoInlineObject.h"
#include "kritatext_export.h"

/**
 * This object is an inline object, which means it is anchored in the text-flow and it can hold
 * bibliography-mark(citation).
 */
class KRITATEXT_EXPORT KoInlineCite : public KoInlineObject
{
    Q_OBJECT
public:
    enum Type {
        Citation,
        ClonedCitation                  //cloned from other citation in document
    };
    /**
     * Construct a new cite to be inserted in the text using KoTextSelectionHandler::insertInlineObject() for example.
     */
    explicit KoInlineCite(Type type);

    virtual ~KoInlineCite();

    bool operator!= (const KoInlineCite &cite) const;

    KoInlineCite &operator= (const KoInlineCite &cite);

    Type type() const;        //return type of cite

    void setType(Type t);

    QString dataField(const QString &fieldName) const;     //returns bibliography-data-field with name fieldName

    bool hasSameData(KoInlineCite *cite) const;

    void copyFrom(KoInlineCite *cite);

    void setIdentifier(const QString &identifier);

    void setBibliographyType(const QString &bibliographyType);

    void setAddress (const QString &addr);

    void setAnnotation (const QString &annotation);

    void setAuthor (const QString &author);

    void setBookTitle (const QString &booktitle);

    void setChapter (const QString &chapter);

    void setEdition (const QString &edition);

    void setEditor (const QString &editor);

    void setPublicationType (const QString &publicationType);

    void setInstitution (const QString &institution);

    void setJournal (const QString &journal);

    void setLabel(const QString &label);

    void setMonth (const QString &month);

    void setNote (const QString &note);

    void setNumber (const QString &number);

    void setOrganisation (const QString &organisation);

    void setPages (const QString &pages);

    void setPublisher (const QString &publisher);

    void setSchool (const QString &school);

    void setSeries (const QString &series);

    void setTitle (const QString &title);

    void setReportType (const QString &reportType);

    void setVolume (const QString &volume);

    void setYear (const QString &year);

    void setURL (const QString &url);

    void setISBN (const QString &isbn);

    void setISSN (const QString &issn);

    void setCustom1 (const QString &custom1);

    void setCustom2 (const QString &custom2);

    void setCustom3 (const QString &custom3);

    void setCustom4 (const QString &custom4);

    void setCustom5 (const QString &custom5);

    QString identifier() const;

    QString address() const;

    QString author() const;

    QString bibliographyType() const;

    QString annotation() const;

    QString bookTitle() const;

    QString chapter() const;

    QString edition() const;

    QString editor() const;

    QString publicationType() const;

    QString institution() const;

    QString journal() const;

    QString month() const;

    QString note() const;

    QString number() const;

    QString organisations() const;

    QString pages() const;

    QString publisher() const;

    QString school() const;

    QString series() const;

    QString title() const;

    QString reportType() const;

    QString volume() const;

    QString year() const;

    QString url() const;

    QString isbn() const;

    QString issn() const;

    QString custom1() const;

    QString custom2() const;

    QString custom3() const;

    QString custom4() const;

    QString custom5() const;

    int posInDocument() const;

    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    ///reimplemented
    void saveOdf(KoShapeSavingContext &context);

protected:
    /// reimplemented
    virtual void updatePosition(const QTextDocument *document, int posInDocument, const QTextCharFormat &format);
    /// reimplemented
    virtual void resize(const QTextDocument *document, QTextInlineObject &object,
                        int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    /// reimplemented
    virtual void paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
                       const QRectF &rect, const QTextInlineObject &object, int posInDocument, const QTextCharFormat &format);

private:
    class Private;
    Private * const d;

};

#endif // KOINLINECITE_H
