/* This file is part of the KDE project
 * Copyright (C) 2007, 2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2009-2010 Thomas Zander <zander@kde.org>
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

#include "BgSpellCheck.h"
#include "SpellCheck.h"

#include <KoCharacterStyle.h>

#include <QTextDocument>
#include <QCoreApplication>
#include <QTextBlock>
#include <KDebug>

#define MaxCharsPerRun 1000

BgSpellCheck::BgSpellCheck(const Speller &speller, QObject *parent):
    BackgroundChecker(speller, parent)
{
    connect(this, SIGNAL(misspelling(const QString &, int)), this, SLOT(foundMisspelling(const QString &, int)));
    QString lang = speller.language();
    if (lang.isEmpty()) // have *some* default...
        lang = "en_US";
    setDefaultLanguage(lang);
}

void BgSpellCheck::setDefaultLanguage(const QString &language)
{
    m_defaultLanguage = language;
    int index = m_defaultLanguage.indexOf('_');
    if (index > 0) {
        m_defaultCountry = m_defaultLanguage.mid(index+1);
        m_defaultLanguage = m_defaultLanguage.left(index);
    }
}

void BgSpellCheck::startRun(QTextDocument *document, int startPosition, int endPosition)
{
    m_document = document;
    m_currentPosition = startPosition;
    m_nextPosition = startPosition;
    m_endPosition = endPosition;
    if (m_currentLanguage != m_defaultLanguage || m_currentCountry != m_defaultCountry) {
        m_currentCountry = m_defaultCountry;
        m_currentLanguage = m_defaultLanguage;
        changeLanguage(m_currentLanguage);
    }
    if (m_currentPosition < m_endPosition) {
        kDebug(31000) << "Starting:" << m_currentPosition << m_endPosition;
        BackgroundChecker::start();
    } else {
        emit done();
    }
}

QString BgSpellCheck::fetchMoreText()
{
    m_currentPosition = m_nextPosition;
    if (m_currentPosition >= m_endPosition)
        return QString();

    QTextBlock block = m_document->findBlock(m_currentPosition);
    if (!block.isValid()) {
        m_nextPosition = m_endPosition; // ends run
        return QString();
    }

    QString language = m_currentLanguage;
    QString country = m_currentCountry;
    QTextCharFormat cf = block.charFormat();
    if (cf.hasProperty(KoCharacterStyle::Language))
        language = cf.property(KoCharacterStyle::Language).toString();
    if (cf.hasProperty(KoCharacterStyle::Country))
        country = cf.property(KoCharacterStyle::Country).toString();

    QTextBlock::iterator iter = block.begin();
    while (!iter.atEnd() && iter.fragment().position() + iter.fragment().length() < m_currentPosition)
        iter++;

    int end = m_endPosition;
    do {
        cf = iter.fragment().charFormat();
        if (cf.hasProperty(KoCharacterStyle::Language))
            language = cf.property(KoCharacterStyle::Language).toString();
        if (cf.hasProperty(KoCharacterStyle::Country))
            country = cf.property(KoCharacterStyle::Country).toString();

        ++iter;
        while (iter.atEnd()) {
            end = block.position() + block.length() - 1;
            block = block.next();
            if (!block.isValid())
                break;
            cf = block.charFormat();
            if (cf.hasProperty(KoCharacterStyle::Language))
                language = cf.property(KoCharacterStyle::Language).toString();
            if (cf.hasProperty(KoCharacterStyle::Country))
                country = cf.property(KoCharacterStyle::Country).toString();
            iter = block.begin();
        }
        if (!block.isValid())
            break;

        Q_ASSERT(!iter.atEnd());
        Q_ASSERT(iter.fragment().isValid());
        end = iter.fragment().position() + iter.fragment().length();
        if (end >= qMin(m_endPosition, m_currentPosition + MaxCharsPerRun))
            break;
    } while(language == m_currentLanguage && country == m_currentCountry);

    if (m_currentLanguage != language || m_currentCountry != country) {
        kDebug(31000) << "switching to language" << language << country;
        // hmm, seems we can't set country. *shrug*
        changeLanguage(language);
        m_currentLanguage = language;
        m_currentCountry = country;
    }

    QTextCursor cursor(m_document);
    cursor.setPosition(end);
    if (end != m_endPosition) {
        cursor.movePosition(QTextCursor::StartOfWord); // word boundary
        end = cursor.position();
    }
    cursor.setPosition(m_currentPosition, QTextCursor::KeepAnchor);
    m_nextPosition = end;
    return cursor.selectedText();
}

void BgSpellCheck::foundMisspelling(const QString &word, int start)
{
    // kDebug(31000) << "Mispelling: " << word << " : " << start;
    emit misspelledWord(word, m_currentPosition + start, true);
    BackgroundChecker::continueChecking();
}

#include <BgSpellCheck.moc>
