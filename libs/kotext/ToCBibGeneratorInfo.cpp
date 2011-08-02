/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>

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
#include "ToCBibGeneratorInfo.h"

IndexEntry::IndexEntry(QString _styleName, IndexEntry::IndexEntryName _name)
        :   styleName(_styleName),
            name(_name)
{

}

IndexEntry::~IndexEntry()
{

}


void IndexEntry::addAttributes(KoXmlWriter* writer) const
{
    Q_UNUSED(writer);
}

void IndexEntry::saveOdf(KoXmlWriter* writer) const
{
    switch (name) {
    case LINK_START:
        writer->startElement("text:index-entry-link-start");
        break;
    case CHAPTER:
        writer->startElement("text:index-entry-chapter");
        break;
    case BIBLIOGRAPHY:
        writer->startElement("text:index-entry-bibliography");
        break;
    case SPAN:
        writer->startElement("text:index-entry-span");
        break;
    case TEXT:
        writer->startElement("text:index-entry-text");
        break;
    case TAB_STOP:
        writer->startElement("text:index-entry-tab-stop");
        break;
    case PAGE_NUMBER:
        writer->startElement("text:index-entry-page-number");
        break;
    case LINK_END:
        writer->startElement("text:index-entry-link-end");
        break;
    case UNKNOWN:
        break;
    }

    if (!styleName.isNull()) {
        writer->addAttribute("text:style-name", styleName);
    }

    addAttributes(writer);
    writer->endElement();
}

IndexEntryBibliography::IndexEntryBibliography(QString _styleName)
    : IndexEntry(_styleName, IndexEntry::BIBLIOGRAPHY)
    , dataField(QString())
{

}


void IndexEntryBibliography::addAttributes(KoXmlWriter* writer) const
{
    if (!dataField.isNull()) {
        writer->addAttribute("text:bibliography-data-field", dataField);
    }
}


IndexEntrySpan::IndexEntrySpan(QString _styleName): IndexEntry(_styleName, IndexEntry::SPAN)
{
    text = QString();
}


void IndexEntrySpan::addAttributes(KoXmlWriter* writer) const
{
    if (!text.isNull() && !text.isEmpty()) {
        writer->addTextNode(text);
    }
}

IndexEntryTabStop::IndexEntryTabStop(QString _styleName): IndexEntry(_styleName, IndexEntry::TAB_STOP)
{

}


void IndexEntryTabStop::addAttributes(KoXmlWriter* writer) const
{
    writer->addAttribute("style:leader-char",tab.leaderText);
    // If the value of this attribute is left, the style:position attribute shall also be present.
    // Otherwise, this attribute shall be omitted.
    if (tab.type == QTextOption::LeftTab) {
        writer->addAttribute("style:type", "left");
        writer->addAttribute("style:position", m_position);
    } else {
        Q_ASSERT(tab.type == QTextOption::RightTab);
        writer->addAttribute("style:type", "right");
    }
}


void IndexEntryTabStop::setPosition(const QString& position)
{
    m_position = position;
    tab.position = KoUnit::parseValue(position);
}

void BibliographyEntryTemplate::saveOdf(KoXmlWriter* writer) const
{
    writer->startElement("text:bibliography-entry-template");
        writer->addAttribute("text:style-name", styleName);
        writer->addAttribute("text:bibliography-type", bibliographyType);
        foreach(IndexEntry* e,indexEntries) {
            e->saveOdf(writer);
        }

    writer->endElement();
}

void IndexTitleTemplate::saveOdf(KoXmlWriter* writer) const
{
    writer->startElement("text:index-title-template");
        writer->addAttribute("text:style-name", styleName);
        if (!text.isEmpty() && !text.isNull()) {
            writer->addTextNode(text);
        }
    writer->endElement();
}

void IndexSourceStyle::saveOdf(KoXmlWriter* writer) const
{
    writer->startElement("text:index-source-styles");
    if (!styleName.isNull()) {
        writer->addAttribute("text:style-name",styleName);
    }
    writer->endElement();
}


void IndexSourceStyles::saveOdf(KoXmlWriter* writer) const
{
    writer->startElement("text:index-source-styles");
        writer->addAttribute("text:outline-level", outlineLevel);
        foreach(const IndexSourceStyle &s, styles) {
            s.saveOdf(writer);
        }
    writer->endElement();
}

IndexEntryPageNumber::IndexEntryPageNumber(QString _styleName): IndexEntry(_styleName, IndexEntry::PAGE_NUMBER)
{

}

IndexEntryLinkEnd::IndexEntryLinkEnd(QString _styleName): IndexEntry(_styleName, IndexEntry::LINK_END)
{

}


void TocEntryTemplate::saveOdf(KoXmlWriter* writer) const
{
    writer->startElement("text:table-of-content-entry-template");
        writer->addAttribute("text:outline-level", outlineLevel);
        writer->addAttribute("text:style-name", styleName);

        foreach(IndexEntry* e,indexEntries) {
            e->saveOdf(writer);
        }

    writer->endElement();
}

IndexEntryText::IndexEntryText(QString _styleName): IndexEntry(_styleName,IndexEntry::TEXT)
{

}

IndexEntryLinkStart::IndexEntryLinkStart(QString _styleName)
    : IndexEntry(_styleName, IndexEntry::LINK_START)
{

}


IndexEntryChapter::IndexEntryChapter(QString _styleName)
    : IndexEntry(_styleName, IndexEntry::CHAPTER)
    , display(QString())
    , outlineLevel(INVALID_OUTLINE_LEVEL)
{

}


void IndexEntryChapter::addAttributes(KoXmlWriter* writer) const
{
    if (!display.isNull()) {
        writer->addAttribute("text:display", display);
    }
    writer->addAttribute("text:outline-level", outlineLevel);
}
