/* This file is part of the  KDE project
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
#include "KoInlineCite.h"

#include "KoInlineTextObjectManager.h"
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoShapeSavingContext.h>
#include <KoTextDocument.h>
#include <KoOdfBibliographyConfiguration.h>
#include <KoStyleManager.h>
#include "TextDebug.h"

#include <QTextDocument>
#include <QTextInlineObject>
#include <QFontMetricsF>
#include <QTextOption>

// Include Q_UNSUSED classes, for building on Windows
#include <KoShapeLoadingContext.h>

class Q_DECL_HIDDEN KoInlineCite::Private
{
public:
    Private(KoInlineCite::Type t)
        : type(t)
    {
    }

    KoInlineCite::Type type;
    int posInDocument;
    QString label;

    QString bibliographyType;
    QString identifier;
    QString address;
    QString annote;
    QString author;
    QString booktitle;
    QString chapter;
    QString edition;
    QString editor;
    QString publicationType;
    QString institution;
    QString journal;
    QString month;
    QString note;
    QString number;
    QString organisation;
    QString pages;
    QString publisher;
    QString school;             //university in UI and saved under text:school tag in XML format
    QString series;
    QString title;
    QString reportType;
    QString volume;
    QString year;
    QString url;
    QString isbn;
    QString issn;
    QString custom1;
    QString custom2;
    QString custom3;
    QString custom4;
    QString custom5;
};

KoInlineCite::KoInlineCite(Type type)
    :KoInlineObject(true)
    ,d(new Private(type))
{
}

KoInlineCite::~KoInlineCite()
{
    delete d;
}

KoInlineCite::Type KoInlineCite::type() const
{
    return d->type;
}

void KoInlineCite::setType(Type t)
{
    d->type = t;
}

QString KoInlineCite::dataField(const QString &fieldName) const
{
    if ( fieldName == "address" ) {
        return d->address;
    } else if ( fieldName == "annote" ) {
        return d->annote;
    } else if ( fieldName == "author" ) {
        return d->author;
    } else if ( fieldName == "bibliography-type" ) {
        return d->bibliographyType;
    } else if ( fieldName == "booktitle" ) {
        return d->booktitle;
    } else if ( fieldName == "chapter" ) {
        return d->chapter;
    } else if ( fieldName == "custom1" ) {
        return d->custom1;
    } else if ( fieldName == "custom2" ) {
        return d->custom2;
    } else if ( fieldName == "custom3" ) {
        return d->custom3;
    } else if ( fieldName == "custom4" ) {
        return d->custom4;
    } else if ( fieldName == "custom5" ) {
        return d->custom5;
    } else if ( fieldName == "edition" ) {
        return d->edition;
    } else if ( fieldName == "editor" ) {
        return d->editor;
    } else if ( fieldName == "howpublished" ) {
        return d->publicationType;
    } else if ( fieldName == "identifier" ) {
        return d->identifier;
    } else if ( fieldName == "institution" ) {
        return d->institution;
    } else if ( fieldName == "isbn" ) {
        return d->isbn;
    } else if ( fieldName == "issn" ) {
        return d->issn;
    } else if ( fieldName == "journal" ) {
        return d->journal;
    } else if ( fieldName == "month" ) {
        return d->month;
    } else if ( fieldName == "note" ) {
        return d->note;
    } else if ( fieldName == "number" ) {
        return d->number;
    } else if ( fieldName == "organisations" ) {
        return d->organisation;
    } else if ( fieldName == "pages" ) {
        return d->pages;
    } else if ( fieldName == "publisher" ) {
        return d->publisher;
    } else if ( fieldName == "report-type" ) {
        return d->reportType;
    } else if ( fieldName == "school" ) {
        return d->school;
    } else if ( fieldName == "series" ) {
        return d->series;
    } else if ( fieldName == "title" ) {
        return d->title;
    } else if ( fieldName == "url" ) {
        return d->url;
    } else if ( fieldName == "volume" ) {
        return d->volume;
    } else if ( fieldName == "year" ) {
        return d->year;
    } else {
        return QString();
    }
}

void KoInlineCite::setIdentifier(const QString &identifier)
{
    d->identifier = identifier;
}

void KoInlineCite::setAddress(const QString &addr)
{
    d->address = addr;
}

void KoInlineCite::setAnnotation(const QString &annotation)
{
    d->annote = annotation;
}

void KoInlineCite::setAuthor(const QString &author)
{
    d->author = author;
}

void KoInlineCite::setBibliographyType(const QString &bibliographyType)
{
    d->bibliographyType = bibliographyType;
}

void KoInlineCite::setBookTitle(const QString &booktitle)
{
    d->booktitle = booktitle;
}

void KoInlineCite::setChapter(const QString &chapter)
{
    d->chapter = chapter;
}

void KoInlineCite::setCustom1(const QString &custom1)
{
    d->custom1 = custom1;
}

void KoInlineCite::setCustom2(const QString &custom2)
{
    d->custom2 = custom2;
}

void KoInlineCite::setCustom3(const QString &custom3)
{
    d->custom3 = custom3;
}

void KoInlineCite::setCustom4(const QString &custom4)
{
    d->custom4 = custom4;
}

void KoInlineCite::setCustom5(const QString &custom5)
{
    d->custom5 = custom5;
}

void KoInlineCite::setEdition(const QString &edition)
{
    d->edition = edition;
}

void KoInlineCite::setEditor(const QString &editor)
{
    d->editor = editor;
}

void KoInlineCite::setInstitution(const QString &institution)
{
    d->institution = institution;
}

void KoInlineCite::setISBN(const QString &isbn)
{
    d->isbn = isbn;
}

void KoInlineCite::setISSN(const QString &issn)
{
    d->issn = issn;
}

void KoInlineCite::setJournal(const QString &journal)
{
    d->journal = journal;
}

void KoInlineCite::setLabel(const QString &label)
{
    d->label = label;
}

void KoInlineCite::setMonth(const QString &month)
{
    d->month = month;
}

void KoInlineCite::setNote(const QString &note)
{
    d->note = note;
}

void KoInlineCite::setNumber(const QString &number)
{
    d->number = number;
}

void KoInlineCite::setOrganisation(const QString &organisation)
{
    d->organisation = organisation;
}

void KoInlineCite::setPages(const QString &pages)
{
    d->pages = pages;
}

void KoInlineCite::setPublicationType(const QString &publicationType)
{
    d->publicationType = publicationType;
}

void KoInlineCite::setPublisher(const QString &publisher)
{
    d->publisher = publisher;
}

void KoInlineCite::setReportType(const QString &reportType)
{
    d->reportType = reportType;
}

void KoInlineCite::setSchool(const QString &school)
{
    d->school = school;
}

void KoInlineCite::setSeries(const QString &series)
{
    d->series = series;
}

void KoInlineCite::setTitle(const QString &title)
{
    d->title = title;
}

void KoInlineCite::setURL(const QString &url)
{
    d->url = url;
}

void KoInlineCite::setVolume(const QString &volume)
{
    d->volume = volume;
}

void KoInlineCite::setYear(const QString &year)
{
    d->year = year;
}

QString KoInlineCite::identifier() const
{
    return d->identifier;
}

QString KoInlineCite::address() const
{
    return d->address;
}

QString KoInlineCite::annotation() const
{
    return d->annote;
}

QString KoInlineCite::author() const
{
    return d->author;
}

QString KoInlineCite::bibliographyType() const
{
    return d->bibliographyType;
}

QString KoInlineCite::bookTitle() const
{
    return d->booktitle;
}

QString KoInlineCite::chapter() const
{
    return d->chapter;
}

QString KoInlineCite::custom1() const
{
    return d->custom1;
}

QString KoInlineCite::custom2() const
{
    return d->custom2;
}

QString KoInlineCite::custom3() const
{
    return d->custom3;
}

QString KoInlineCite::custom4() const
{
    return d->custom4;
}

QString KoInlineCite::custom5() const
{
    return d->custom5;
}

QString KoInlineCite::edition() const
{
    return d->edition;
}

QString KoInlineCite::editor() const
{
    return d->editor;
}

QString KoInlineCite::institution() const
{
    return d->institution;
}

QString KoInlineCite::isbn() const
{
    return d->isbn;
}

QString KoInlineCite::issn() const
{
    return d->issn;
}

QString KoInlineCite::journal() const
{
    return d->journal;
}

QString KoInlineCite::month() const
{
    return d->month;
}

QString KoInlineCite::note() const
{
    return d->note;
}

QString KoInlineCite::number() const
{
    return d->number;
}

QString KoInlineCite::organisations() const
{
    return d->organisation;
}

QString KoInlineCite::pages() const
{
    return d->pages;
}

QString KoInlineCite::publisher() const
{
    return d->publisher;
}

QString KoInlineCite::publicationType() const
{
    return d->publicationType;
}

QString KoInlineCite::reportType() const
{
    return d->reportType;
}

QString KoInlineCite::school() const
{
    return d->school;
}

QString KoInlineCite::series() const
{
    return d->series;
}

QString KoInlineCite::title() const
{
    return d->title;
}

QString KoInlineCite::volume() const
{
    return d->volume;
}

QString KoInlineCite::year() const
{
    return d->year;
}

QString KoInlineCite::url() const
{
    return d->url;
}

int KoInlineCite::posInDocument() const
{
    return d->posInDocument;
}

bool KoInlineCite::operator!= ( const KoInlineCite &cite ) const
{
    return !(d->address == cite.address() && d->annote == cite.annotation() && d->author == cite.author() &&
            d->bibliographyType == cite.bibliographyType() && d->booktitle == cite.bookTitle() &&
            d->chapter == cite.chapter() && d->custom1 == cite.custom1() && d->custom2 == cite.custom2() &&
            d->custom3 == cite.custom3() && d->custom4 == cite.custom4() && d->custom5 == cite.custom5() &&
            d->edition == cite.edition() && d->editor == cite.editor() && d->identifier == cite.identifier() &&
            d->institution == cite.institution() && d->isbn == cite.isbn() && d->issn == cite.issn() &&
            d->journal == cite.journal() && d->month == cite.month() && d->note == cite.note() &&
            d->number == cite.number() && d->organisation == cite.organisations() && d->pages == cite.pages() &&
            d->publicationType == cite.publicationType() && d->publisher == cite.publisher() &&
            d->reportType == cite.reportType() && d->school == cite.school() && d->series == cite.series() &&
            d->title == cite.title() && d->url == cite.url() && d->volume == cite.volume() && d->year == cite.year());
}

KoInlineCite &KoInlineCite::operator =(const  KoInlineCite &cite)
{
    d->address = cite.address();
    d->annote = cite.annotation();
    d->author = cite.author();
    d->bibliographyType = cite.bibliographyType();
    d->booktitle = cite.bookTitle();
    d->chapter = cite.chapter();
    d->custom1 = cite.custom1();
    d->custom2 = cite.custom2();
    d->custom3 = cite.custom3();
    d->custom4 = cite.custom4();
    d->custom5 = cite.custom5();
    d->edition = cite.edition();
    d->editor = cite.editor();
    d->identifier = cite.identifier();
    d->institution = cite.institution();
    d->isbn = cite.isbn();
    d->issn = cite.issn();
    d->journal = cite.journal();
    d->month = cite.month();
    d->note = cite.note();
    d->number = cite.number();
    d->organisation = cite.organisations();
    d->pages = cite.pages();
    d->publicationType = cite.publicationType();
    d->publisher = cite.publisher();
    d->reportType = cite.reportType();
    d->school = cite.school();
    d->series = cite.series();
    d->title = cite.title();
    d->url = cite.url();
    d->volume = cite.volume();
    d->year = cite.year();

    return *this;
}

void KoInlineCite::updatePosition(const QTextDocument *document, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(document);
    Q_UNUSED(format);
    d->posInDocument = posInDocument;
}

void KoInlineCite::resize(const QTextDocument *document, QTextInlineObject &object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
    if (d->identifier.isEmpty())
        return;

    KoOdfBibliographyConfiguration *bibConfiguration = KoTextDocument(document).styleManager()->bibliographyConfiguration();

    if (!bibConfiguration->numberedEntries()) {
        d->label = QString("%1%2%3").arg(bibConfiguration->prefix())
                                    .arg(d->identifier)
                                    .arg(bibConfiguration->suffix());
    } else {
        d->label = QString("%1%2%3").arg(bibConfiguration->prefix())
                    .arg(QString::number(manager()->citationsSortedByPosition(true).indexOf(this) + 1))
                    .arg(bibConfiguration->suffix());
    }

    Q_ASSERT(format.isCharFormat());
    QFontMetricsF fm(format.font(), pd);
    object.setWidth(fm.width(d->label));
    object.setAscent(fm.ascent());
    object.setDescent(fm.descent());
}

void KoInlineCite::paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document, const QRectF &rect, const QTextInlineObject &object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);

    if (d->identifier.isEmpty())
        return;

    KoOdfBibliographyConfiguration *bibConfiguration = KoTextDocument(document).styleManager()->bibliographyConfiguration();

    if (!bibConfiguration->numberedEntries()) {
        d->label = QString("%1%2%3").arg(bibConfiguration->prefix())
                                    .arg(d->identifier)
                                    .arg(bibConfiguration->suffix());
    } else {
        d->label = QString("%1%2%3").arg(bibConfiguration->prefix())
                    .arg(QString::number(manager()->citationsSortedByPosition(true, document->firstBlock()).indexOf(this) + 1))
                    .arg(bibConfiguration->suffix());
    }

    QFont font(format.font(), pd);
    QTextLayout layout(d->label, font, pd);
    layout.setCacheEnabled(true);
    QList<QTextLayout::FormatRange> layouts;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = d->label.length();
    range.format = format;
    range.format.setVerticalAlignment(QTextCharFormat::AlignNormal);
    layouts.append(range);
    layout.setAdditionalFormats(layouts);

    QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
    option.setTextDirection(object.textDirection());
    layout.setTextOption(option);
    layout.beginLayout();
    layout.createLine();
    layout.endLayout();
    layout.draw(&painter, rect.topLeft());
}

bool KoInlineCite::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(context);
    //KoTextLoader loader(context);
    if (element.namespaceURI() == KoXmlNS::text && element.localName() == "bibliography-mark") {
        d->identifier = element.attributeNS(KoXmlNS::text, "identifier");
        d->bibliographyType = element.attributeNS(KoXmlNS::text, "bibliography-type");
        d->address = element.attributeNS(KoXmlNS::text, "address");
        d->annote = element.attributeNS(KoXmlNS::text, "annote");
        d->author = element.attributeNS(KoXmlNS::text, "author");
        d->booktitle = element.attributeNS(KoXmlNS::text, "booktitle");
        d->chapter = element.attributeNS(KoXmlNS::text, "chapter");
        d->edition = element.attributeNS(KoXmlNS::text, "edition");
        d->editor = element.attributeNS(KoXmlNS::text, "editor");
        d->publicationType = element.attributeNS(KoXmlNS::text, "howpublished");
        d->institution = element.attributeNS(KoXmlNS::text, "institution");
        d->journal = element.attributeNS(KoXmlNS::text, "journal");
        d->month = element.attributeNS(KoXmlNS::text, "month");
        d->note = element.attributeNS(KoXmlNS::text, "note");
        d->number = element.attributeNS(KoXmlNS::text, "number");
        d->organisation = element.attributeNS(KoXmlNS::text, "organisations");
        d->pages = element.attributeNS(KoXmlNS::text, "pages");
        d->publisher = element.attributeNS(KoXmlNS::text, "publisher");
        d->school = element.attributeNS(KoXmlNS::text, "school");
        d->series = element.attributeNS(KoXmlNS::text, "series");
        d->title = element.attributeNS(KoXmlNS::text, "title");
        d->reportType = element.attributeNS(KoXmlNS::text, "report-type");
        d->volume = element.attributeNS(KoXmlNS::text, "volume");
        d->year = element.attributeNS(KoXmlNS::text, "year");
        d->url = element.attributeNS(KoXmlNS::text, "url");
        d->isbn = element.attributeNS(KoXmlNS::text, "isbn");
        d->issn = element.attributeNS(KoXmlNS::text, "issn");
        d->custom1 = element.attributeNS(KoXmlNS::text, "custom1");
        d->custom2 = element.attributeNS(KoXmlNS::text, "custom2");
        d->custom3 = element.attributeNS(KoXmlNS::text, "custom3");
        d->custom4 = element.attributeNS(KoXmlNS::text, "custom4");
        d->custom5 = element.attributeNS(KoXmlNS::text, "custom5");

        //Now checking for cloned citation (with same identifier)
        if (manager()->citations(true).keys().count(d->identifier) > 1) {
            this->setType(KoInlineCite::ClonedCitation);
        }
    }
    else {
        return false;
    }
    return true;
}

void KoInlineCite::saveOdf(KoShapeSavingContext &context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    writer->startElement("text:bibliography-mark", false);

    if (!d->identifier.isEmpty())
        writer->addAttribute("text:identifier", d->identifier);     //can't be "" //to be changed later
    if (!d->bibliographyType.isEmpty())
        writer->addAttribute("text:bibliography-type", d->bibliographyType);
    if (!d->address.isEmpty())
        writer->addAttribute("text:address",d->identifier);
    if (!d->annote.isEmpty())
        writer->addAttribute("text:annote",d->annote);
    if (!d->author.isEmpty())
        writer->addAttribute("text:author",d->author);
    if (!d->booktitle.isEmpty())
        writer->addAttribute("text:booktitle",d->booktitle);
    if (!d->chapter.isEmpty())
        writer->addAttribute("text:chapter",d->chapter);
    if (!d->edition.isEmpty())
        writer->addAttribute("text:edition",d->edition);
    if (!d->editor.isEmpty())
        writer->addAttribute("text:editor",d->editor);
    if (!d->publicationType.isEmpty())
        writer->addAttribute("text:howpublished",d->publicationType);
    if (!d->institution.isEmpty())
        writer->addAttribute("text:institution",d->institution);
    if (!d->journal.isEmpty())
        writer->addAttribute("text:journal",d->journal);
    if (!d->month.isEmpty())
        writer->addAttribute("text:month",d->month);
    if (!d->note.isEmpty())
        writer->addAttribute("text:note",d->note);
    if (!d->number.isEmpty())
        writer->addAttribute("text:number",d->number);
    if (!d->pages.isEmpty())
        writer->addAttribute("text:pages",d->pages);
    if (!d->publisher.isEmpty())
        writer->addAttribute("text:publisher",d->publisher);
    if (!d->school.isEmpty())
        writer->addAttribute("text:school",d->school);
    if (!d->series.isEmpty())
        writer->addAttribute("text:series",d->series);
    if (!d->title.isEmpty())
        writer->addAttribute("text:title",d->title);
    if (!d->reportType.isEmpty())
        writer->addAttribute("text:report-type",d->reportType);
    if (!d->volume.isEmpty())
        writer->addAttribute("text:volume",d->volume);
    if (!d->year.isEmpty())
        writer->addAttribute("text:year",d->year);
    if (!d->url.isEmpty())
        writer->addAttribute("text:url",d->url);
    if (!d->isbn.isEmpty())
        writer->addAttribute("text:isbn",d->isbn);
    if (!d->issn.isEmpty())
        writer->addAttribute("text:issn",d->issn);
    if (!d->custom1.isEmpty())
        writer->addAttribute("text:custom1",d->custom1);
    if (!d->custom2.isEmpty())
        writer->addAttribute("text:custom2",d->custom2);
    if (!d->custom3.isEmpty())
        writer->addAttribute("text:custom3",d->custom3);
    if (!d->custom4.isEmpty())
        writer->addAttribute("text:custom4",d->custom4);
    if (!d->custom5.isEmpty())
        writer->addAttribute("text:custom5",d->custom5);

    writer->addTextNode(QString("[%1]").arg(d->identifier));
    writer->endElement();
}
