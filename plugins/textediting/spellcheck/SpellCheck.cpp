/* This file is part of the KDE project
 * Copyright (C) 2007, 2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
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
#include <QTextLayout>
#include <QTextCharFormat>

#include <KLocale>
#include <KDebug>
#include <KAction>
#include <sonnet/configdialog.h>

#include <KoCharacterStyle.h>
#include <KoResourceManager.h>

#include "BgSpellCheck.h"

SpellCheck::SpellCheck()
    : m_bgSpellCheck(0),
    m_enableSpellCheck(true),
    m_allowSignals(true),
    m_documentIsLoading(false)
{
    /* setup actions for this plugin */
    KAction *configureAction = new KAction(i18n("Configure &Spell Checking..."), this);
    connect(configureAction, SIGNAL(triggered()), this, SLOT(configureSpellCheck()));
    addAction("tool_configure_spellcheck", configureAction);

    KConfigGroup spellConfig = KGlobal::config()->group("Spelling");
    m_speller = Sonnet::Speller(spellConfig.readEntry("defaultLanguage", "en_US"));
    m_bgSpellCheck = new BgSpellCheck(m_speller, this);

    m_defaultMisspelledFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    m_defaultMisspelledFormat.setUnderlineColor(QColor(Qt::red)); // TODO make use kde-config

    connect(m_bgSpellCheck, SIGNAL(misspelledWord(const QString &,int,bool)),
            this, SLOT(highlightMisspelled(const QString &,int,bool)));
    connect(m_bgSpellCheck, SIGNAL(done()), this, SLOT(dequeueDocument()));
}

void SpellCheck::finishedWord(QTextDocument *document, int cursorPosition)
{
    setDocument(document);
    if (m_documentIsLoading || !m_enableSpellCheck)
        return;

    QTextBlock block = document->findBlock(cursorPosition);
    if (!block.isValid())
        return;
    QList<QTextLayout::FormatRange> ranges = block.layout()->additionalFormats();
    QList<QTextLayout::FormatRange> newRanges;
    QList<QTextLayout::FormatRange>::Iterator iter = ranges.begin();
    while (iter != ranges.end()) {
        QTextLayout::FormatRange r = *(iter);
        if (r.format != m_defaultMisspelledFormat) // any ranges in the block at higher cursor pos can be removed.
            newRanges.append(r);
        ++iter;
    }
    m_allowSignals = false;
    if (ranges.count() != newRanges.count())
        block.layout()->setAdditionalFormats(newRanges);
    m_allowSignals = true;
    m_bgSpellCheck->startRun(m_document, block.position(), block.position() + block.length());
}

void SpellCheck::finishedParagraph(QTextDocument *document, int cursorPosition)
{
    Q_UNUSED(document);
    Q_UNUSED(cursorPosition);
}

void SpellCheck::checkSection(QTextDocument *document, int startPosition, int endPosition)
{
    setDocument(document);
    if (m_documentIsLoading || !m_enableSpellCheck) return;

    if (m_documentsQueue.isEmpty()) {
        kDebug(31000) << "Document queue is empty";
        m_bgSpellCheck->startRun(document, startPosition, endPosition);
    } else {
        m_documentsQueue.enqueue(document);
    }
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

void SpellCheck::setDefaultLanguage(const QString &language)
{
    m_speller.setDefaultLanguage(language);
    m_bgSpellCheck->setDefaultLanguage(language);
}

void SpellCheck::setBackgroundSpellChecking(bool on)
{
    if (m_enableSpellCheck == on)
        return;
    m_enableSpellCheck = on;
    if (!m_enableSpellCheck) {
        // TODO remove all misspellings.
    }
}

void SpellCheck::setSkipAllUppercaseWords(bool on)
{
    m_speller.setAttribute(Speller::CheckUppercase, !on);
}

void SpellCheck::setSkipRunTogetherWords(bool on)
{
    m_speller.setAttribute(Speller::SkipRunTogether, on);
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
    return !m_documentIsLoading && m_enableSpellCheck;
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
    kDebug(31000) << "Highlighting: " << word << " : " << startPosition;
    QTextBlock block = m_document->findBlock(startPosition);
    QTextLayout *layout = block.layout();
    QTextLayout::FormatRange range;
    range.start = startPosition - block.position();
    range.length = word.trimmed().length();

    QList<QTextLayout::FormatRange> ranges = layout->additionalFormats();
    range.format = m_defaultMisspelledFormat;
    ranges.append(range);
    m_allowSignals = false;
    if (ranges.isEmpty())
        layout->clearAdditionalFormats();
    else if (ranges.count() != layout->additionalFormats().count())
        layout->setAdditionalFormats(ranges);
    if (misspelled)
        m_document->markContentsDirty(startPosition, range.length);
    m_allowSignals = true;
}

void SpellCheck::dequeueDocument()
{
    // kDebug(31000) << "Dequeueing document";
    if (!m_documentsQueue.isEmpty()) {
        m_bgSpellCheck->startRun(m_documentsQueue.dequeue(), 0, 0);
    }
}

void SpellCheck::checkDocument(int position, int charsRemoved, int charsAdded)
{
    if (! m_allowSignals)
        return;

    int changeStart = position;
    int changeEnd = position;
    bool recheck = false;

    // TODO make for loop for all affected blocks ??
    QTextBlock block = m_document->findBlock(position);
    QTextLayout *layout = block.layout();
    QList< QTextLayout::FormatRange > ranges = layout->additionalFormats();
    for (int index =0; index < ranges.count(); index++) {
        QTextLayout::FormatRange range = ranges[index];
        if (range.format != m_defaultMisspelledFormat)
            continue;

        if (range.start > position - block.position()
                && qMax(charsRemoved, charsAdded) + position - block.position() < range.start + range.length) { // range is moved.
            range.start += charsAdded - charsRemoved;
        }
        else if (position - block.position() >= range.start &&
                position - block.position() < range.start + range.length) { // change is starting inside this range
            if (position - block.position() + qMax(charsRemoved, charsAdded) > range.start + range.length) {
                recheck = true;
                changeEnd = qMax(changeEnd, range.start + range.length);
            }

            range.length = 0; // delete range, I don't want to see spell-check errors while typing a word
        } else {
            continue;
        }

        ranges.takeAt(index);
        if (range.length > 0) {
            ranges.insert(0, range);
            changeStart = qMin(changeStart, range.start);
            changeEnd = qMax(changeEnd, range.start + range.length);
        } else {
            index--; // make sure we don't skip one
        }
    }

    m_allowSignals = false;
    layout->setAdditionalFormats(ranges);
    if (changeStart != changeEnd)
        m_document->markContentsDirty(changeStart, changeEnd);
    m_allowSignals = true;

    if (recheck)
        m_bgSpellCheck->startRun(m_document, changeStart, changeEnd);
}

void SpellCheck::setDocument(QTextDocument *document)
{
    if (m_document.data() == document) return;
    if (!m_document.isNull())
        disconnect(m_document, SIGNAL(contentsChange(int,int,int)), this, SLOT(checkDocument(int,int,int)));
    m_document = document;
    connect(m_document, SIGNAL(contentsChange(int,int,int)), this, SLOT(checkDocument(int,int,int)), Qt::DirectConnection);
}

void SpellCheck::configureSpellCheck()
{
    Sonnet::ConfigDialog *dialog = new Sonnet::ConfigDialog(KGlobal::config().data(), 0);
    connect (dialog, SIGNAL(languageChanged(const QString&)), this, SLOT(setDefaultLanguage(const QString&)));
    dialog->exec();
    delete dialog;
}

void SpellCheck::resourceChanged(int key, const QVariant &resource)
{
    if (key == KoCanvasResource::DocumentIsLoading)
        m_documentIsLoading = resource.toBool();
}

#include <SpellCheck.moc>
