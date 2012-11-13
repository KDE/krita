/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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
#ifndef AUTOCORRECT_H
#define AUTOCORRECT_H

#include <KoTextEditingPlugin.h>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QSet>

/* struct AutocorrectEntry
{
    QString replace;
    QTextCharFormat format;
}; */

class Autocorrect : public KoTextEditingPlugin {
    Q_OBJECT

public:
    struct TypographicQuotes {
        QChar begin;
        QChar end;
    };

    Autocorrect();
    virtual ~Autocorrect();

    void finishedWord(QTextDocument *document, int cursorPosition);
    void finishedParagraph(QTextDocument *document, int cursorPosition);

    void setUppercaseFirstCharOfSentence(bool b) { m_uppercaseFirstCharOfSentence = b; }
    void setFixTwoUppercaseChars(bool b) { m_fixTwoUppercaseChars = b; }
    void setAutoFormatURLs(bool b) { m_autoFormatURLs = b; }
    void setSingleSpaces(bool b) { m_singleSpaces = b; }
    void setTrimParagraphs(bool b) { m_trimParagraphs = b; }
    void setAutoBoldUnderline(bool b) { m_autoBoldUnderline = b; }
    void setAutoFractions(bool b) { m_autoFractions = b; }
    void setAutoNumbering(bool b) { m_autoNumbering = b; }
    void setSuperscriptAppendix(bool b) { m_superscriptAppendix = b; }
    void setCapitalizeWeekDays(bool b) { m_capitalizeWeekDays = b; }
    void setAutoFormatBulletList(bool b) { m_autoFormatBulletList = b; }
    void setReplaceDoubleQuotes(bool b) { m_replaceDoubleQuotes = b; }
    void setReplaceSingleQuotes(bool b) { m_replaceSingleQuotes = b; }
    void setAdvancedAutocorrect(bool b) { m_advancedAutocorrect = b; }
    void setTypographicSingleQuotes(const TypographicQuotes& singleQuote) { m_typographicSingleQuotes = singleQuote; }
    void setTypographicDoubleQuotes(const TypographicQuotes& doubleQuote) { m_typographicDoubleQuotes = doubleQuote; }
    void setUpperCaseExceptions(const QSet<QString>& exceptions);
    void setTwoUpperLetterExceptions(const QSet<QString>& exceptions);
    void setAutocorrectEntries(const QHash<QString, QString>& entries);

    bool getUppercaseFirstCharOfSentence() const { return m_uppercaseFirstCharOfSentence; }
    bool getFixTwoUppercaseChars()const  { return m_fixTwoUppercaseChars; }
    bool getAutoFormatURLs() const { return m_autoFormatURLs; }
    bool getSingleSpaces() const { return m_singleSpaces; }
    bool getTrimParagraphs() const { return m_trimParagraphs; }
    bool getAutoBoldUnderline() const { return m_autoBoldUnderline; }
    bool getAutoFractions() const { return m_autoFractions; }
    bool getAutoNumbering() const { return m_autoNumbering; }
    bool getSuperscriptAppendix() const { return m_superscriptAppendix; }
    bool getCapitalizeWeekDays() const { return m_capitalizeWeekDays; }
    bool getAutoFormatBulletList() const { return m_autoFormatBulletList; }
    bool getReplaceDoubleQuotes() const { return m_replaceDoubleQuotes; }
    bool getReplaceSingleQuotes() const { return m_replaceSingleQuotes; }
    bool getAdvancedAutocorrect() const { return m_advancedAutocorrect; }
    TypographicQuotes getTypographicSingleQuotes() const { return m_typographicSingleQuotes; }
    TypographicQuotes getTypographicDoubleQuotes() const { return m_typographicDoubleQuotes; }
    TypographicQuotes getTypographicDefaultSingleQuotes() const;
    TypographicQuotes getTypographicDefaultDoubleQuotes() const;
    QSet<QString> getUpperCaseExceptions() const;
    QSet<QString> getTwoUpperLetterExceptions() const;
    QHash<QString, QString> getAutocorrectEntries() const;

private slots:
    void configureAutocorrect();

private:
    void uppercaseFirstCharOfSentence();
    void fixTwoUppercaseChars();
    /// @returns true if processing should stop here.
    bool autoFormatURLs();
    /// @returns true if processing should stop here.
    bool singleSpaces();
    /// @returns true if processing should stop here.
    bool autoBoldUnderline();
    /// @returns true if processing should stop here.
    bool autoFractions();
    void autoNumbering();
    void superscriptAppendix();
    void capitalizeWeekDays();
    void autoFormatBulletList();
    void replaceTypographicQuotes();
    void advancedAutocorrect();

    /// @returns the actual link that will be set as anchor href
    QString autoDetectURL(const QString &word) const;

    void readConfig();
    void writeConfig();
    void readAutocorrectXmlEntries();
    void readAutocorrectXmlEntry(const QString &fname, bool onlyCustomization);
    void writeAutocorrectXmlEntry();

private:
    bool m_uppercaseFirstCharOfSentence; // convert first letter of a sentence automaticall to uppercase
    bool m_fixTwoUppercaseChars;  // convert two uppercase characters to one upper and one lowercase.
    bool m_autoFormatURLs;
    bool m_singleSpaces; // suppress double spaces.
    bool m_trimParagraphs; // remove spaces at beginning and end of paragraphs
    bool m_autoBoldUnderline; // automatically do bold and underline formatting
    bool m_autoFractions; // replace 1/2 with ½
    bool m_autoNumbering; //use autonumbering for numbered paragraphs
    bool m_superscriptAppendix; // replace 1st with 1 and a superscript "st"
    bool m_capitalizeWeekDays;
    bool m_autoFormatBulletList; // use list formatting for bulletted paragraphs.
    bool m_advancedAutocorrect; // autocorrection from a list of entries

    bool m_replaceDoubleQuotes;  // replace double quotes with typographical quotes
    bool m_replaceSingleQuotes;  // replace single quotes with typographical quotes

    QString m_word;
    QTextCursor m_cursor;

    QString m_autocorrectLang;
    QStringList m_cacheNameOfDays;
    QHash<QString, QString> m_superScriptEntries;
    QSet<QString> m_upperCaseExceptions;
    QSet<QString> m_twoUpperLetterExceptions;
    QHash<QString, QString /*AutocorrectEntry*/> m_autocorrectEntries;
    TypographicQuotes m_typographicSingleQuotes;
    TypographicQuotes m_typographicDoubleQuotes;
    KAction *m_enabled;
};

#endif
