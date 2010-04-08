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
#ifndef KOODFNOTESCONFIGURATION_H
#define KOODFNOTESCONFIGURATION_H

#include <QString>
#include <QPair>
#include <QMetaType>

#include "KoXmlReader.h"
#include "KoXmlWriter.h"
#include "KoOdfNumberDefinition.h"
#include "koodf_export.h"
/**
 * load and save the notes-configuration element from the text: namespace.
 *
 * @see 14.9.2 Notes Configuration Element
 * A document in OpenDocument format contains at most one notes configuration element for every
 * notes class used in the document. If there is no note configuration element, a default note
 * configuration is used.
 * The attributes that may be associated with the <text:notes-configuration> element are:
 *
 * • Note class
 * • Citation text style
 * • Citation body text style
 * • Default footnote paragraph style
 * • Master page
 * • Start value
 * • Number format
 * • Numbering scheme
 * • Footnote position
 *
 * The following element may be included in the <text:footnotes-configuration> element:
 *
 * • Footnote continuation notice (forward and backward)
 */
class KOODF_EXPORT KoOdfNotesConfiguration
{
public:

    KoOdfNotesConfiguration();
    ~KoOdfNotesConfiguration();
    KoOdfNotesConfiguration(const KoOdfNotesConfiguration &other);
    KoOdfNotesConfiguration &operator=(const KoOdfNotesConfiguration &other);


    /**
     * load the notes-configuration element
     */
    void loadOdf(const KoXmlElement &element);

    /**
     * save the notes-configuration element
     */
    void saveOdf(KoXmlWriter * writer) const;

    /**
     * Note class
     * The note class attribute determines which note elements this notes configuration applies to.
     */
    enum NoteClass {
        Footnote,
        Endnote,
        Unknown
    };

    NoteClass noteClass() const;
    void setNoteClass(NoteClass noteClass);

    /**
     * Citation Text Style
     * The text:citation-style attribute specifies the text style to use for the footnote citation
     * within the footnote.
     */
    QString citationTextStyle() const;
    void setCitationTextStyle(const QString &citationTextStyle);

    /**
     * Citation Body Text Style
     * The text:citation-body-style-name attribute specifies the text style to use for the
     * footnote citation in the text flow.
     */
    QString citationBodyTextStyle() const;
    void setCitationBodyTextStyle(const QString &citationBodyTextStyle);

    /**
     * Default Note Paragraph Style
     * The default footnote paragraph style is only used for footnotes that are inserted into an existing
     * document. It is not used for footnotes that already exist.
     */
    QString defaultNoteParagraphStyle() const;
    void setDefaultNoteParagraphStyle(const QString &defaultNoteParagraphStyle);

    /**
     * Master Page
     * To display the footnotes at the end of the document, the pages that contain the footnotes must be
     * instances of the master page specified by the text:master-page-name attribute.
     */
    QString masterPage() const;
    void setMasterPage(const QString &masterPage);

    /**
     * Start Value
     * The start:value attribute specifies the value at which the footnote numbering starts.
     */
    int startValue() const;
    void setStartValue(int startValue);

    /**
     * Number Format
     * See section 12.2 for information on the number format for footnotes.
     */
    KoOdfNumberDefinition numberFormat() const;
    void setNumberFormat(const KoOdfNumberDefinition &numberFormat);

    /**
     * Numbering Scheme
     * The text:start-numbering-at attribute specifies if footnote numbers start with a new
     * number at the beginning of the document or at the beginning of each chapter or page.
     */
    enum NumberingScheme {
        BeginAtDocument,
        BeginAtChapter,
        BeginAtPage
    };

    NumberingScheme numberingScheme() const;
    void setNumberingScheme(NumberingScheme numberingScheme);

    /**
     * Footnotes Position
     * • The text:footnotes-position attribute specifies one of the following positions for footnotes:
     * • text: At the page where the footnote citation is located, immediately below the page's text.
     * • page: The bottom of the page where the footnote citation is located.
     * • section: The end of the section
     * • document: The end of the document.
     */
    enum FootnotesPosition {
        Text,
        Page,
        Section,
        Document
    };

    FootnotesPosition footnotesPosition() const;
    void setFootnotesPosition(FootnotesPosition footnotesPosition);

    /**
     * Footnote Continuation
     *  The footnote continuation elements specify:
     *   • Text displayed at the end of a footnote that is continued on the next page
     *   • Text displayed before the continued text
     */
    QString footnoteContinuationForward() const;
    void setFootnoteContinuationForward(const QString &footnoteContinuationForward);

    QString footnoteContinuationBackward() const;
    void setFootnoteContinuationBackward(const QString &footnoteContinuationBackward);

private:

    class Private;
    Private * const d;

};

Q_DECLARE_METATYPE(KoOdfNotesConfiguration*)

#endif // KOODFNOTESCONFIGURATION_H
