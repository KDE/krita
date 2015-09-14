/* This file is part of the KDE project

   Copyright (C) 2010 Boudewijn Rempt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
#include "KoOdfNotesConfiguration.h"

#include <OdfDebug.h>
#include "KoXmlNS.h"
#include "KoXmlWriter.h"
#include "KoOdfNumberDefinition.h"

class Q_DECL_HIDDEN KoOdfNotesConfiguration::Private
{
public:
    KoOdfNotesConfiguration::NoteClass noteClass;
    QString citationTextStyleName;
    QString citationBodyTextStyleName;
    QString defaultNoteParagraphStyleName;
    void *citationTextStyle;
    void *citationBodyTextStyle;
    void *defaultNoteParagraphStyle;
    QString masterPageName;
    int startValue;
    KoOdfNumberDefinition numberFormat;
    KoOdfNotesConfiguration::NumberingScheme numberingScheme;
    KoOdfNotesConfiguration::FootnotesPosition footnotesPosition;
    QString footnotesContinuationForward;
    QString footnotesContinuationBackward;

};

KoOdfNotesConfiguration::KoOdfNotesConfiguration(NoteClass noteClass)
    : d(new Private())
{
    d->noteClass = noteClass;
    d->startValue = 1;
    d->numberingScheme = BeginAtDocument;
    d->footnotesPosition = Page;

    d->defaultNoteParagraphStyle = 0;
    d->citationTextStyle = 0;
    d->citationBodyTextStyle = 0;

    if (noteClass == KoOdfNotesConfiguration::Footnote) {
        d->numberFormat.setFormatSpecification(KoOdfNumberDefinition::Numeric);
        d->defaultNoteParagraphStyleName = "Footnote";
        d->citationTextStyleName = "Footnote_20_Symbol";
        d->citationBodyTextStyleName = "Footnote_20_anchor";
    } else {
        d->numberFormat.setFormatSpecification(KoOdfNumberDefinition::RomanLowerCase);
        d->defaultNoteParagraphStyleName = "Endnote";
        d->citationTextStyleName = "Endnote_20_Symbol";
        d->citationBodyTextStyleName = "Endnote_20_anchor";
    }
}

KoOdfNotesConfiguration::~KoOdfNotesConfiguration()
{
    delete d;
}

KoOdfNotesConfiguration::KoOdfNotesConfiguration(const KoOdfNotesConfiguration &other)
    : QObject(), d(new Private())
{
    d->noteClass = other.d->noteClass;
    d->citationTextStyleName = other.d->citationTextStyleName;
    d->citationBodyTextStyleName = other.d->citationBodyTextStyleName;
    d->defaultNoteParagraphStyleName = other.d->defaultNoteParagraphStyleName;
    d->citationTextStyle = other.d->citationTextStyle;
    d->citationBodyTextStyle = other.d->citationBodyTextStyle;
    d->defaultNoteParagraphStyle = other.d->defaultNoteParagraphStyle;
    d->masterPageName = other.d->masterPageName;
    d->startValue = other.d->startValue;
    d->numberFormat = other.d->numberFormat;
    d->numberingScheme = other.d->numberingScheme;
    d->footnotesPosition = other.d->footnotesPosition;
    d->footnotesContinuationForward = other.d->footnotesContinuationForward;
    d->footnotesContinuationBackward = other.d->footnotesContinuationBackward;

}

KoOdfNotesConfiguration &KoOdfNotesConfiguration::operator=(const KoOdfNotesConfiguration &other)
{
    d->noteClass = other.d->noteClass;
    d->citationTextStyleName = other.d->citationTextStyleName;
    d->citationBodyTextStyleName = other.d->citationBodyTextStyleName;
    d->defaultNoteParagraphStyleName = other.d->defaultNoteParagraphStyleName;
    d->citationTextStyle = other.d->citationTextStyle;
    d->citationBodyTextStyle = other.d->citationBodyTextStyle;
    d->defaultNoteParagraphStyle = other.d->defaultNoteParagraphStyle;
    d->masterPageName = other.d->masterPageName;
    d->startValue = other.d->startValue;
    d->numberFormat = other.d->numberFormat;
    d->numberingScheme = other.d->numberingScheme;
    d->footnotesPosition = other.d->footnotesPosition;
    d->footnotesContinuationForward = other.d->footnotesContinuationForward;
    d->footnotesContinuationBackward = other.d->footnotesContinuationBackward;

    return *this;
}


void KoOdfNotesConfiguration::loadOdf(const KoXmlElement &element)
{
    d->citationTextStyleName = element.attributeNS(KoXmlNS::text, "citation-style-name", d->citationTextStyleName);
    d->citationBodyTextStyleName = element.attributeNS(KoXmlNS::text, "citation-body-style-name", d->citationBodyTextStyleName);
    d->defaultNoteParagraphStyleName = element.attributeNS(KoXmlNS::text, "default-style-name", d->defaultNoteParagraphStyleName);
    d->masterPageName = element.attributeNS(KoXmlNS::text, "master-page-name", d->masterPageName);
    d->startValue = qMax(1, element.attributeNS(KoXmlNS::text, "start-value", QString::number(d->startValue)).toInt());

    d->numberFormat.loadOdf(element);

    QString numberingScheme = element.attributeNS(KoXmlNS::text, "start-numbering-at", "document");
    if (numberingScheme == "document") {
        d->numberingScheme = BeginAtDocument;
    }
    else if (numberingScheme == "chapter") {
        d->numberingScheme = BeginAtChapter;
    }
    else if (numberingScheme == "page") {
        d->numberingScheme = BeginAtPage;
    }

    QString footnotesPosition  = element.attributeNS(KoXmlNS::text, "footnotes-position", "page");
    if (footnotesPosition == "text") {
        d->footnotesPosition = Text;
    }
    else if (footnotesPosition == "page") {
        d->footnotesPosition = Page;
    }
    else if (footnotesPosition == "section") {
        d->footnotesPosition = Section;
    }
    else if (footnotesPosition == "document") {
        d->footnotesPosition = Document;
    }

    for (KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        KoXmlElement child = node.toElement();
        if (child.namespaceURI() == KoXmlNS::text) {
            if (child.localName() == "note-continuation-notice-forward") {
                d->footnotesContinuationForward = child.text();
            } else if (child.localName() == "note-continuation-notice-backward") {
                d->footnotesContinuationBackward = child.text();
            }
        }
    }
}

void KoOdfNotesConfiguration::saveOdf(KoXmlWriter *writer) const
{
    writer->startElement("text:notes-configuration");

    if (d->noteClass == Footnote) {
        writer->addAttribute("text:note-class", "footnote");
    }
    else if (d->noteClass == Endnote) {
        writer->addAttribute("text:note-class", "endnote");
    }
    if (!d->citationTextStyleName.isNull()) {writer->addAttribute("text:citation-style-name", d->citationTextStyleName); }
    if (!d->citationBodyTextStyleName.isNull()) {writer->addAttribute("text:citation-body-style-name", d->citationBodyTextStyleName); }
    if (!d->defaultNoteParagraphStyleName.isNull()) {writer->addAttribute("text:default-style-name", d->defaultNoteParagraphStyleName); }
    if (!d->masterPageName.isNull()) {writer->addAttribute("text:master-page-name", d->masterPageName); }
    if (d->startValue != 0) { writer->addAttribute("text:start-value", d->startValue); }

    d->numberFormat.saveOdf(writer);
    switch(d->numberingScheme) {
    case BeginAtDocument:
        writer->addAttribute("text:start-numbering-at", "document");
        break;
    case BeginAtChapter:
        writer->addAttribute("text:start-numbering-at", "chapter");
        break;
    case BeginAtPage:
        writer->addAttribute("text:start-numbering-at", "page");
        break;
    }
    switch(d->footnotesPosition) {
    case Text:
        writer->addAttribute("text:footnotes-position", "text");
        break;
    case Page:
        writer->addAttribute("text:footnotes-position", "page");
        break;
    case Section:
        writer->addAttribute("text:footnotes-position", "section");
        break;
    case Document:
        writer->addAttribute("text:footnotes-position", "document");
        break;
    }
    if (!d->footnotesContinuationForward.isNull()) {
        writer->startElement("text:note-continuation-notice-forward", false);
        writer->addTextNode(d->footnotesContinuationForward);
        writer->endElement();
    }
    if (!d->footnotesContinuationBackward.isNull()) {
        writer->startElement("text:note-continuation-notice-backward", false);
        writer->addTextNode(d->footnotesContinuationBackward);
        writer->endElement();
    }

    writer->endElement(); //text:notes-configuration
}


KoOdfNotesConfiguration::NoteClass KoOdfNotesConfiguration::noteClass() const
{
    return d->noteClass;
}


void *KoOdfNotesConfiguration::citationTextStyle() const
{
    return d->citationTextStyle;
}

QString KoOdfNotesConfiguration::citationTextStyleName() const
{
    return d->citationTextStyleName;
}

void KoOdfNotesConfiguration::setCitationTextStyle(void *citationTextStyle)
{
    d->citationTextStyle = citationTextStyle;
}

void *KoOdfNotesConfiguration::citationBodyTextStyle() const
{
    return d->citationBodyTextStyle;
}

QString KoOdfNotesConfiguration::citationBodyTextStyleName() const
{
    return d->citationBodyTextStyleName;
}

void KoOdfNotesConfiguration::setCitationBodyTextStyle(void *citationBodyTextStyle)
{
    d->citationBodyTextStyle = citationBodyTextStyle;
}

void *KoOdfNotesConfiguration::defaultNoteParagraphStyle() const
{
    return d->defaultNoteParagraphStyle;
}

QString KoOdfNotesConfiguration::defaultNoteParagraphStyleName() const
{
    return d->defaultNoteParagraphStyleName;
}

void KoOdfNotesConfiguration::setDefaultNoteParagraphStyle(void *defaultNoteParagraphStyle)
{
    d->defaultNoteParagraphStyle = defaultNoteParagraphStyle;
}

QString KoOdfNotesConfiguration::masterPage() const
{
    return d->masterPageName;
}

void KoOdfNotesConfiguration::setMasterPage(const QString &masterPage)
{
    d->masterPageName = masterPage;
}

int KoOdfNotesConfiguration::startValue() const
{
    return d->startValue;
}

void KoOdfNotesConfiguration::setStartValue(int startValue)
{
    d->startValue = qMax(1, startValue);
}


KoOdfNumberDefinition KoOdfNotesConfiguration::numberFormat() const
{
    return d->numberFormat;
}

void KoOdfNotesConfiguration::setNumberFormat(const KoOdfNumberDefinition &numberFormat)
{
    d->numberFormat = numberFormat;
}

KoOdfNotesConfiguration::NumberingScheme KoOdfNotesConfiguration::numberingScheme() const
{
    return d->numberingScheme;
}

void KoOdfNotesConfiguration::setNumberingScheme(NumberingScheme numberingScheme)
{
    d->numberingScheme = numberingScheme;
}

KoOdfNotesConfiguration::FootnotesPosition KoOdfNotesConfiguration::footnotesPosition() const
{
    return d->footnotesPosition;
}

void KoOdfNotesConfiguration::setFootnotesPosition(FootnotesPosition footnotesPosition)
{
    d->footnotesPosition = footnotesPosition;
}

QString KoOdfNotesConfiguration::footnoteContinuationForward() const
{
    return d->footnotesContinuationForward;
}

void KoOdfNotesConfiguration::setFootnoteContinuationForward(const QString &footnoteContinuationForward)
{
    d->footnotesContinuationForward = footnoteContinuationForward;
}

QString KoOdfNotesConfiguration::footnoteContinuationBackward() const
{
    return d->footnotesContinuationBackward;
}

void KoOdfNotesConfiguration::setFootnoteContinuationBackward(const QString &footnoteContinuationBackward)
{
    d->footnotesContinuationBackward = footnoteContinuationBackward;
}
