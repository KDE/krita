/* This file is part of the KDE project
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

#include "SpellCheck.h"

#include <QTextBlock>
#include <QTextDocument>

#include <KLocale>
#include <KDebug>

#include <KoCharacterStyle.h>

#include "BgSpellCheck.h"
#include "SpellCheckConfigDialog.h"

SpellCheck::SpellCheck()
{
    /* setup actions for this plugin */
    QAction *configureAction = new QAction(i18n("Configure &Spell Checking..."), this);
    connect(configureAction, SIGNAL(triggered()), this, SLOT(configureSpellCheck()));
    addAction("tool_configure_spellcheck", configureAction);

    m_speller = Sonnet::Speller("en_US"); // TODO: make it configurable
    m_bgSpellCheck = new BgSpellCheck(m_speller, this);
    m_enableSpellCheck = true;
    m_document = 0;

    connect(m_bgSpellCheck, SIGNAL(misspelledWord(const QString &,int,bool)), this, SLOT(highlightMisspelled(const QString &,int,bool)));
    connect(m_bgSpellCheck, SIGNAL(done()), this, SLOT(dequeueDocument()));
}

void SpellCheck::finishedWord(QTextDocument *document, int cursorPosition)
{
    if (!m_enableSpellCheck) return;

    Q_UNUSED(document);

    m_cursor.setPosition(cursorPosition);
    m_cursor.movePosition(QTextCursor::WordLeft);
    m_cursor.select(QTextCursor::WordUnderCursor);
    int startPosition = m_cursor.selectionStart();
    int endPosition;

    if (m_cursor.movePosition(QTextCursor::NextWord))
        m_cursor.select(QTextCursor::WordUnderCursor);
    endPosition = m_cursor.selectionEnd();

    /* clean up the old spelling result, so that white space doesn't copy char format from previous cursor position */
    m_cursor.setPosition(startPosition);
    m_cursor.setPosition(endPosition, QTextCursor::KeepAnchor);
    QTextCharFormat format;
    format.setProperty(KoCharacterStyle::Spelling, false);
    m_cursor.mergeCharFormat(format);

    m_bgSpellCheck->start(m_document, startPosition, endPosition);
}

void SpellCheck::finishedParagraph(QTextDocument *document, int cursorPosition)
{
    Q_UNUSED(document);
    Q_UNUSED(cursorPosition);
}

void SpellCheck::checkSection(QTextDocument *document, int startPosition, int endPosition)
{
    if (!m_enableSpellCheck) return;

    // we are not asked to do spellchecking, just update our current document
    if (startPosition == 0 && endPosition == 0) {
        if (m_document)
            disconnect(m_document, SIGNAL(contentsChange(int,int,int)), this, SLOT(checkDocument(int,int,int)));
        m_document = document;
        connect(m_document, SIGNAL(contentsChange(int,int,int)), this, SLOT(checkDocument(int,int,int)));
    }

    m_cursor = QTextCursor(document);
    if (m_documentsQueue.isEmpty()) {
        kDebug(31000) << "Document queue is empty";
        m_bgSpellCheck->start(document, startPosition, endPosition);
    }
    else
        m_documentsQueue.enqueue(document);
}

QStringList SpellCheck::availableBackends() const
{
    return m_speller.availableBackends();
}

QStringList SpellCheck::availableLanguages() const
{
    return m_speller.availableLanguages();
}

/* void SpellCheck::setDefaultClient(const QString &client)
{
    m_speller.setDefaultClient(client);
} */

void SpellCheck::setDefaultLanguage(const QString &lang)
{
    m_speller.setDefaultLanguage(lang);
}

void SpellCheck::setBackgroundSpellChecking(bool b)
{
    m_enableSpellCheck = b;
}

void SpellCheck::setSkipAllUppercaseWords(bool b)
{
    m_speller.setAttribute(Speller::CheckUppercase, !b);
}

void SpellCheck::setSkipRunTogetherWords(bool b)
{
    m_speller.setAttribute(Speller::SkipRunTogether, b);
}

/* QString SpellCheck::defaultClient() const
{
    return m_speller.defaultClient();
} */

QString SpellCheck::defaultLanguage() const
{
    return m_speller.defaultLanguage();
}

bool SpellCheck::backgroundSpellChecking()
{
    return m_enableSpellCheck;
}

bool SpellCheck::skipAllUppercaseWords()
{
    return m_speller.testAttribute(Speller::CheckUppercase);
}

bool SpellCheck::skipRunTogetherWords()
{
    return m_speller.testAttribute(Speller::SkipRunTogether);
}

void SpellCheck::highlightMisspelled(const QString &word, int startPosition, bool misspelled)
{
    // TODO: Exception, such as words that contain numbers, numbers
    // kDebug(31000) << "Highlighting: " << word << " : " << startPosition;
    m_document->blockSignals(true);
    m_cursor.setPosition(startPosition);
    m_cursor.setPosition(startPosition + word.trimmed().length(), QTextCursor::KeepAnchor);

    QTextCharFormat format;
    format.setProperty(KoCharacterStyle::Spelling, misspelled);
    m_cursor.mergeCharFormat(format);

    m_document->blockSignals(false);
}

void SpellCheck::dequeueDocument()
{
    // kDebug(31000) << "Dequeueing document";
    if (!m_documentsQueue.isEmpty()) {
        m_bgSpellCheck->start(m_documentsQueue.dequeue(), 0, 0);
    }
}

void SpellCheck::checkDocument(int position, int charsRemoved, int charsAdded)
{
    if (charsAdded == 1 && charsRemoved == 0) return;

    m_cursor.setPosition(position);
    m_cursor.movePosition(QTextCursor::WordLeft);
    int startPosition = m_cursor.position();
    m_cursor.setPosition(position + charsAdded - charsRemoved);
    m_cursor.movePosition(QTextCursor::WordRight);
    int endPosition = m_cursor.position();

    m_bgSpellCheck->start(m_document, startPosition, endPosition);
    // kDebug() << "start position: " << startPosition << " end position: " << endPosition;
}

void SpellCheck::configureSpellCheck()
{
    SpellCheckConfigDialog *cfgDlg = new SpellCheckConfigDialog(this);
    if (cfgDlg->exec()) {
        // TODO
    }
}

#include "SpellCheck.moc"
