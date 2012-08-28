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

BgSpellCheck::BgSpellCheck(QObject *parent)
    : BackgroundChecker(parent)
{
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
        if (m_currentCountry.isEmpty()) {
            changeLanguage(m_currentLanguage);
        } else {
            changeLanguage(m_currentLanguage+'_'+m_currentCountry);
        }
    }
    if (m_currentPosition < m_endPosition) {
        kDebug(31000) << "Starting:" << m_currentPosition << m_endPosition;
        start();
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
    QTextBlock::iterator iter;
    while (true) {
        if (!block.isValid()) {
            m_nextPosition = m_endPosition; // ends run
            return QString();
        }
        if (block.length() == 1) { // only linefeed
            block = block.next();
            m_currentPosition++;
            continue;
        }

        iter = block.begin();
        while (!iter.atEnd() && iter.fragment().position() + iter.fragment().length() <=
                m_currentPosition)
            ++iter;
        break;
    }

    int end = m_endPosition;
    QTextCharFormat cf = iter.fragment().charFormat();
    QString language;
    if (cf.hasProperty(KoCharacterStyle::Language))
        language = cf.property(KoCharacterStyle::Language).toString();
    else
        language = m_defaultLanguage;
    QString country;
    if (cf.hasProperty(KoCharacterStyle::Country))
        country = cf.property(KoCharacterStyle::Country).toString();
    else
        country = m_defaultCountry;

    // qDebug() << "init" << language << country << "/" << iter.fragment().position();
    while(true) {
        end = iter.fragment().position() + iter.fragment().length();
        // qDebug() << " + " << iter.fragment().position() << "-" << iter.fragment().position() + iter.fragment().length()
            // << block.text().mid(iter.fragment().position() - block.position(), iter.fragment().length());
        if (end >= qMin(m_endPosition, m_currentPosition + MaxCharsPerRun))
            break;
        if (!iter.atEnd())
            ++iter;
        if (iter.atEnd()) { // end of block.
            m_nextPosition = block.position() + block.length();
            end = m_nextPosition - 1;
            break;
        }
        Q_ASSERT(iter.fragment().isValid());
        // qDebug() << "Checking for viability forwarding to " << iter.fragment().position();
        cf = iter.fragment().charFormat();
        // qDebug() << " new fragment language;"
            // << (cf.hasProperty(KoCharacterStyle::Language) ?  cf.property(KoCharacterStyle::Language).toString() : "unset");

        if ((cf.hasProperty(KoCharacterStyle::Language)
                    && language != cf.property(KoCharacterStyle::Language).toString())
                || (!cf.hasProperty(KoCharacterStyle::Language)
                    && language != m_defaultLanguage))
            break;

        if ((cf.hasProperty(KoCharacterStyle::Country)
                    && country != cf.property(KoCharacterStyle::Country).toString())
                || (!cf.hasProperty(KoCharacterStyle::Country)
                    && country != m_defaultCountry))
            break;
    }

    if (m_currentLanguage != language || m_currentCountry != country) {
        kDebug(31000) << "switching to language" << language << country;
        m_currentLanguage = language;
        m_currentCountry = country;
#if 0
     Disabling this as sonnet crashes on this. See https://bugs.kde.org/228271
        if (m_currentCountry.isEmpty()) {
            changeLanguage(m_currentLanguage);
        } else {
            changeLanguage(m_currentLanguage+'_'+m_currentCountry);
        }

#endif
    }

    QTextCursor cursor(m_document);
    cursor.setPosition(end);
    cursor.setPosition(m_currentPosition, QTextCursor::KeepAnchor);
    if (m_nextPosition < end)
        m_nextPosition = end;
    return cursor.selectedText();
}

void BgSpellCheck::foundMisspelling(const QString &word, int start)
{
    // kDebug(31000) << "Mispelling: " << word << " : " << start;
    emit misspelledWord(word, m_currentPosition + start, true);
    BackgroundChecker::continueChecking();
}

QString BgSpellCheck::currentLanguage()
{
    return m_currentLanguage;
}

#include <BgSpellCheck.moc>
