/* This file is part of the KDE project
 * Copyright (C) 2007, 2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2007,2009,2010 Thomas Zander <zander@kde.org>
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
#include "BgSpellCheck.h"

#include <KoCharacterStyle.h>
#include <KoResourceManager.h>

#include <KLocale>
#include <KDebug>
#include <KAction>
#include <sonnet/configdialog.h>

#include <QTextBlock>
#include <QThread>
#include <QTimer>
#include <QApplication>
#include <QTextCharFormat>

SpellCheck::SpellCheck()
    : m_bgSpellCheck(0),
    m_enableSpellCheck(true),
    m_allowSignals(true),
    m_documentIsLoading(false),
    m_isChecking(false)
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
    connect(m_bgSpellCheck, SIGNAL(done()), this, SLOT(finishedRun()));
}

void SpellCheck::finishedWord(QTextDocument *document, int cursorPosition)
{
    if (m_documentIsLoading || !m_enableSpellCheck)
        return;

    QTextBlock block = document->findBlock(cursorPosition);
    if (!block.isValid())
        return;
    checkSection(document, block.position(), block.position() + block.length());
}

void SpellCheck::finishedParagraph(QTextDocument *document, int cursorPosition)
{
    Q_UNUSED(document);
    Q_UNUSED(cursorPosition);
    //QTextBlock block = document->findBlock(cursorPosition);
    //checkSection(document, block.position(), block.position() + block.length());
}

void SpellCheck::checkSection(QTextDocument *document, int startPosition, int endPosition)
{
    if (m_documentIsLoading || !m_enableSpellCheck) return;

    foreach (const SpellSections &ss, m_documentsQueue) {
        if (ss.from <= startPosition && ss.to >= endPosition)
            return;
        // TODO also check if we should replace an existing queued item with a longer span
    }

    SpellSections ss(document, startPosition, endPosition);
    m_documentsQueue.enqueue(ss);
    runQueue();
}

QStringList SpellCheck::availableBackends() const
{
    return m_speller.availableBackends();
}

QStringList SpellCheck::availableLanguages() const
{
    return m_speller.availableLanguages();
}

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
    // be aware that this method is called in a different thread!!
    if (!misspelled)
        return;

#if 0
    // DEBUG
class MyThread : public QThread { public: static void mySleep(unsigned long msecs) { msleep(msecs); }};
static_cast<MyThread*>(QThread::currentThread())->mySleep(400);
#endif

    int blockIndex = 0;
    while (blockIndex < m_misspellings.count()) {
        BlockLayout bl = m_misspellings[blockIndex];
        if (bl.start <= startPosition && bl.start + bl.length > startPosition) {
            break;
        }
        ++blockIndex;
    }
    if (blockIndex >= m_misspellings.count()) // not found, doc went out of sync
        return;
    BlockLayout block = m_misspellings.at(blockIndex);

    QTextLayout::FormatRange range;
    range.format = m_defaultMisspelledFormat;
    range.start = startPosition - block.start;
    range.length = word.trimmed().length();
    block.ranges << range;
    m_misspellings[blockIndex] = block;
}

void SpellCheck::runQueue()
{
    Q_ASSERT(QThread::currentThread() == QApplication::instance()->thread());
    if (m_isChecking)
        return;
    while (!m_documentsQueue.isEmpty()) {
        SpellSections section = m_documentsQueue.dequeue();
        if (section.document.isNull())
            continue;
        QTextBlock block = section.document->findBlock(section.from);
        if (!block.isValid())
            continue;
        m_isChecking = true;
        m_document = section.document;
        m_misspellings.clear();
        do {
            BlockLayout bl;
            bl.start = block.position();
            bl.length = block.length();
            bl.checkStart = qMax(bl.start, section.from);
            m_misspellings << bl;
            block = block.next();
        } while(block.isValid() && block.position() < section.to);

        m_bgSpellCheck->startRun(section.document, section.from, section.to);
        break;
    }
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

void SpellCheck::finishedRun()
{
    Q_ASSERT(QThread::currentThread() == QApplication::instance()->thread());
    m_isChecking = false;

    m_allowSignals = false;
    foreach (const BlockLayout &bl, m_misspellings) {
        QTextBlock block = m_document->findBlock(bl.start);
        if (!block.isValid())
            continue;
        if (bl.start != block.position() || bl.length != block.length())
            continue;
        QList<QTextLayout::FormatRange> ranges = block.layout()->additionalFormats();
        bool changed = false;
        int numMisspellings = 0;
        foreach (const QTextLayout::FormatRange &range, ranges) {
            if (range.format != m_defaultMisspelledFormat)
                continue;
            if (range.start + range.length < bl.checkStart)
                continue;
            ++numMisspellings;
            bool found = false;
            foreach (const QTextLayout::FormatRange &newRange, bl.ranges) {
                if (range.start == newRange.start && range.length == newRange.length) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                changed = true;
                break;
            }
        }
        if (changed || numMisspellings != bl.ranges.count()) {
            QList<QTextLayout::FormatRange> newRanges;
            newRanges = bl.ranges;
            foreach (const QTextLayout::FormatRange &range, ranges) {
                if (range.format != m_defaultMisspelledFormat
                        || range.start + range.length < bl.checkStart)
                    newRanges << range;
            }
            if (newRanges.isEmpty())
                block.layout()->clearAdditionalFormats();
            else
                block.layout()->setAdditionalFormats(newRanges);
            m_document->markContentsDirty(bl.start, bl.start + bl.length);
        }
    }
    m_allowSignals = true;

    QTimer::singleShot(0, this, SLOT(runQueue()));
}

#include <SpellCheck.moc>
