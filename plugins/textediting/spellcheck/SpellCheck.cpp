/* This file is part of the KDE project
 * Copyright (C) 2007, 2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2007,2009,2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Christoph Goerlich <chgoerlich@gmx.de>
 * Copyright (C) 2012 Shreya Pandit <shreya@shreyapandit.com>
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
#include "SpellCheckMenu.h"

#include <KoCharacterStyle.h>

#include <KLocale>
#include <KDebug>
#include <KAction>
#include <KToggleAction>
#include <sonnet/configdialog.h>

#include <QTextBlock>
#include <QThread>
#include <QTimer>
#include <QApplication>
#include <QTextCharFormat>

SpellCheck::SpellCheck()
    : m_document(0),
    m_bgSpellCheck(0),
    m_enableSpellCheck(true),
    m_allowSignals(true),
    m_documentIsLoading(false),
    m_isChecking(false),
    m_spellCheckMenu(0)
{
    /* setup actions for this plugin */
    KAction *configureAction = new KAction(i18n("Configure &Spell Checking..."), this);
    connect(configureAction, SIGNAL(triggered()), this, SLOT(configureSpellCheck()));
    addAction("tool_configure_spellcheck", configureAction);

    KToggleAction *spellCheck = new KToggleAction(i18n("Auto Spell Check"), this);
    addAction("tool_auto_spellcheck", spellCheck);

    KConfigGroup spellConfig = KGlobal::config()->group("Spelling");
    m_enableSpellCheck = spellConfig.readEntry("autoSpellCheck", m_enableSpellCheck);
    spellCheck->setChecked(m_enableSpellCheck);
    m_speller = Sonnet::Speller(spellConfig.readEntry("defaultLanguage", "en_US"));
    m_bgSpellCheck = new BgSpellCheck(m_speller, this);

    m_defaultMisspelledFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    m_defaultMisspelledFormat.setUnderlineColor(QColor(Qt::red)); // TODO make use kde-config


    m_spellCheckMenu = new SpellCheckMenu(m_speller, this);
    QPair<QString, KAction*> pair = m_spellCheckMenu->menuAction();
    addAction(pair.first, pair.second);

    connect(m_bgSpellCheck, SIGNAL(misspelledWord(const QString &,int,bool)),
            this, SLOT(highlightMisspelled(const QString &,int,bool)));
    connect(m_bgSpellCheck, SIGNAL(done()), this, SLOT(finishedRun()));
    connect(spellCheck, SIGNAL(toggled(bool)), this, SLOT(setBackgroundSpellChecking(bool)));
    connect(m_spellCheckMenu, SIGNAL(clearHighlightingForWord(int)),
            this, SLOT(clearHighlightMisspelled(int)));
}

void SpellCheck::finishedWord(QTextDocument *document, int cursorPosition)
{
    setDocument(document);
    if (!m_enableSpellCheck)
        return;

    QTextBlock block = document->findBlock(cursorPosition);
    if (!block.isValid())
        return;
    checkSection(document, block.position(), block.position() + block.length() - 1);
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
    setDocument(document);
    if (!m_enableSpellCheck)
        return;
    if (startPosition >= endPosition) // no work
        return;

    foreach (const SpellSections &ss, m_documentsQueue) {
        if (ss.from <= startPosition && ss.to >= endPosition)
            return;
        // TODO also check if we should replace an existing queued item with a longer span
    }

    SpellSections ss(document, startPosition, endPosition);
    m_documentsQueue.enqueue(ss);
    runQueue();
    m_spellCheckMenu->setVisible(true);
}

void SpellCheck::setDocument(QTextDocument *document)
{
    if (m_document == document)
        return;
    if (m_document)
        disconnect (document, SIGNAL(contentsChange(int,int,int)), this, SLOT(documentChanged(int,int,int)));

    m_document = document;
    connect (document, SIGNAL(contentsChange(int,int,int)), this, SLOT(documentChanged(int,int,int)));
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
    if (m_enableSpellCheck && m_document) {
        checkSection(m_document, 0, m_document->characterCount() - 1);
    }
}

void SpellCheck::setBackgroundSpellChecking(bool on)
{
    if (m_enableSpellCheck == on)
        return;
    KConfigGroup spellConfig = KGlobal::config()->group("Spelling");
    m_enableSpellCheck = on;
    spellConfig.writeEntry("autoSpellCheck", m_enableSpellCheck);
    if (m_document) {
        if (!m_enableSpellCheck) {
            for (QTextBlock block = m_document->begin(); block != m_document->end(); block = block.next()) {
                if (block.isValid() && block.layout()->additionalFormats().count() > 0) {
                    block.layout()->clearAdditionalFormats();
                    m_document->markContentsDirty(block.position(), block.position() + block.length());
                }
            }
            m_spellCheckMenu->setEnabled(false);
            m_spellCheckMenu->setVisible(false);
        } else {
            //when re-enabling 'Auto Spell Check' we want spellchecking the whole document
            checkSection(m_document, 0, m_document->characterCount() - 1);
            m_spellCheckMenu->setVisible(true);
        }
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

bool SpellCheck::addWordToPersonal(const QString &word)
{
    return m_bgSpellCheck->addWordToPersonal(word);
}


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

void SpellCheck::documentChanged(int from, int min, int plus)
{
    if (min == plus)
        return;
    if (m_isChecking)
        return;
    QTextDocument *document = qobject_cast<QTextDocument*>(sender());
    if (document == 0)
        return;
    QTextBlock block = document->findBlock(from);
    if (!block.isValid())
        return;
    QList<QTextLayout::FormatRange> ranges = block.layout()->additionalFormats();
    bool changed = false;
    for (int i=0; i < ranges.count(); ++i) {
        const QTextLayout::FormatRange &range = ranges.at(i);
        if (block.position() + range.start > from && range.format == m_defaultMisspelledFormat) {
            QTextLayout::FormatRange newRange = range;
            newRange.start += plus - min;
            ranges[i] = newRange;
            changed = true;
        } else if ((block.position() + range.start > from || block.position() + range.start + range.length > from)
                && range.format == m_defaultMisspelledFormat) {
            ranges.removeAt(i);
            --i;
            changed = true;
        }
    }
    if (changed) {
        block.layout()->setAdditionalFormats(ranges);
    }
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
                        || range.start + range.length < bl.checkStart) {
                    //workaround to avoid multiple adding of already existing ranges in AdditionalFormats of the TextBlock
                    bool rangeAlreadyExists= false;
                    foreach (const QTextLayout::FormatRange &newRange, newRanges) {
                        if (newRange.start == range.start && range.format == m_defaultMisspelledFormat) {
                            rangeAlreadyExists = true;
                            break;
                        }
                    }
                    if (!rangeAlreadyExists)
                        newRanges << range;
                }
            }
            if (newRanges.isEmpty())
                block.layout()->clearAdditionalFormats();
            else
                block.layout()->setAdditionalFormats(newRanges);
            m_document->markContentsDirty(bl.start, bl.length);
        }
    }
    m_allowSignals = true;

    QTimer::singleShot(0, this, SLOT(runQueue()));
}

void SpellCheck::setCurrentCursorPosition(QTextDocument *document, int cursorPosition)
{
    setDocument(document);
    if (m_enableSpellCheck) {
        //check if word at cursor is misspelled
        QTextBlock block = m_document->findBlock(cursorPosition);
        if (block.isValid() && block.layout()->additionalFormats().count() > 0) {
            QList<QTextLayout::FormatRange> ranges = block.layout()->additionalFormats();
            foreach (const QTextLayout::FormatRange &range, ranges) {
                if (cursorPosition >= block.position() + range.start
                        && cursorPosition <= block.position() + range.start + range.length
                        && range.format == m_defaultMisspelledFormat) {
                    QString word = block.text().mid(range.start, range.length);
                    m_spellCheckMenu->setMisspelled(word, block.position() + range.start,range.length);
                    m_spellCheckMenu->setCurrentLanguage(m_bgSpellCheck->currentLanguage());
                    m_spellCheckMenu->setVisible(true);
                    m_spellCheckMenu->setEnabled(true);
                    return;
                }
            }
            m_spellCheckMenu->setEnabled(false);
        } else {
            m_spellCheckMenu->setEnabled(false);
        }
    }
}

void SpellCheck::clearHighlightMisspelled(int startPosition)
{
    if (!m_document)
        return;

    QTextBlock block = m_document->findBlock(startPosition);
    if (!block.isValid())
        return;

    QList<QTextLayout::FormatRange> ranges = block.layout()->additionalFormats();
    bool found = false;
    for (int i = 0; i < ranges.count(); ++i) {
        const QTextLayout::FormatRange &range = ranges.at(i);
        if (startPosition == block.position() + range.start
                && range.format == m_defaultMisspelledFormat) {
            ranges.removeAt(i);
            found = true;
            break;
        }
    }
    if (found) {
        block.layout()->setAdditionalFormats(ranges);
        m_document->markContentsDirty(block.position(), block.length());
    }
}

void SpellCheck::replaceWordBySuggestion(const QString &word, int startPosition, int lengthOfWord)
{
    if (!m_document)
        return;

    QTextBlock block = m_document->findBlock(startPosition);
    if (!block.isValid())
        return;

    QTextCursor cursor(m_document);
    cursor.setPosition(startPosition);
    cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor, lengthOfWord);
    cursor.removeSelectedText();
    //if the replaced word and the suggestion had the same number of chars,
    //we must clear highlighting manually, see 'documentChanged'
    if ((cursor.selectionEnd() - cursor.selectionStart()) == word.length())
        clearHighlightMisspelled(startPosition);

    cursor.insertText(word);
}

#include <SpellCheck.moc>
