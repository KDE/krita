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

#include <KoXmlNS.h>
#include "KoUnit.h"

class KoOdfNotesConfiguration::Private
{
public:
    KoOdfNotesConfiguration::NoteClass noteClass;
    QString citationTextStyle;
    QString citationBodyTextStyle;
    QString defaultNoteParagraphStyle;
    QString masterPageName;
    int startValue;
    KoOdfNumberDefinition numberFormat;
    KoOdfNotesConfiguration::NumberingScheme numberingScheme;
    KoOdfNotesConfiguration::FootnotesPosition footnotesPosition;
    QString footnotesContinuationForward;
    QString footnotesContinuationBackward;

};

KoOdfNotesConfiguration::KoOdfNotesConfiguration()
    : d(new Private())
{
    d->noteClass = Unknown;
    d->startValue = 1;
    d->numberingScheme = BeginAtDocument;
    d->footnotesPosition = Page;
}

KoOdfNotesConfiguration::~KoOdfNotesConfiguration()
{
    delete d;
}

void KoOdfNotesConfiguration::loadOdf(const KoXmlElement &element)
{
    KoXmlElement properties(KoXml::namedItemNS(element, KoXmlNS::text,
                                                "notes-configuration"));
    QString noteClass = properties.attributeNS(KoXmlNS::text, "note-class", "footnote");
    if (noteClass == "footnote") {
        d->noteClass = Footnote;
    }
    else if (noteClass == "document") {
        d->noteClass = Endnote;
    }

    d->citationTextStyle = properties.attributeNS(KoXmlNS::text, "citation-style-name", QString::null);
    d->citationBodyTextStyle = properties.attributeNS(KoXmlNS::text, "citation-body-style-name", QString::null);
    d->defaultNoteParagraphStyle = properties.attributeNS(KoXmlNS::text, "default-style-name", QString::null);
    d->masterPageName = properties.attributeNS(KoXmlNS::text, "master-page-name", QString::null);
    d->startValue = KoUnit::parseValue(properties.attributeNS(KoXmlNS::text, "start-value", "0"));

    d->numberFormat.loadOdf(properties);

    QString numberingScheme = properties.attributeNS(KoXmlNS::text, "start-numbering-at", "document");
    if (numberingScheme == "document") {
        d->numberingScheme = BeginAtDocument;
    }
    else if (numberingScheme == "chapter") {
        d->numberingScheme = BeginAtChapter;
    }
    else if (numberingScheme == "page") {
        d->numberingScheme = BeginAtPage;
    }

    QString footnotesPosition  = properties.attributeNS(KoXmlNS::text, "footnotes-position", "document");
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

    d->footnotesContinuationForward = properties.attributeNS(KoXmlNS::text, "note-continuation-notice-forward", "...");
    d->footnotesContinuationBackward = properties.attributeNS(KoXmlNS::text, "note-continuation-notice-backward", "...");

}

void KoOdfNotesConfiguration::saveOdf(KoXmlWriter *writer) const
{
    Q_UNUSED(writer);
#ifdef __GNUC__
    #warning Implement saving of text:notes-configuration
#endif

}


KoOdfNotesConfiguration::NoteClass KoOdfNotesConfiguration::noteClass() const
{
    return d->noteClass;
}

void KoOdfNotesConfiguration::setNoteClass(NoteClass noteClass)
{
    d->noteClass = noteClass;
}


QString KoOdfNotesConfiguration::citationTextStyle() const
{
    return d->citationTextStyle;
}

void KoOdfNotesConfiguration::setCitationTextStyle(const QString &citationTextStyle)
{
    d->citationTextStyle = citationTextStyle;
}


QString KoOdfNotesConfiguration::citationBodyTextStyle() const
{
    return d->citationBodyTextStyle;
}

void KoOdfNotesConfiguration::setCitationBodyTextStyle(const QString &citationBodyTextStyle)
{
    d->citationBodyTextStyle = citationBodyTextStyle;
}

QString KoOdfNotesConfiguration::defaultNoteParagraphStyle() const
{
    return d->defaultNoteParagraphStyle;
}

void KoOdfNotesConfiguration::setDefaultNoteParagraphStyle(const QString &defaultNoteParagraphStyle)
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
    d->startValue = startValue;
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
