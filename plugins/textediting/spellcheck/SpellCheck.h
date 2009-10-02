/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef SPELLCHECK_H
#define SPELLCHECK_H

#include <QQueue>

#include <KoTextEditingPlugin.h>

#include <sonnet/speller.h>
#include <QTextCharFormat>
class QTextDocument;
class BgSpellCheck;

class SpellCheck : public KoTextEditingPlugin
{
    Q_OBJECT
public:
    SpellCheck();

    void finishedWord(QTextDocument *document, int cursorPosition);
    void finishedParagraph(QTextDocument *document, int cursorPosition);
    void checkSection(QTextDocument *document, int startPosition, int endPosition);

    QStringList availableBackends() const;
    QStringList availableLanguages() const;

    // void setDefaultClient(const QString &client);
    void setBackgroundSpellChecking(bool b);
    void setSkipAllUppercaseWords(bool b);
    void setSkipRunTogetherWords(bool b);

    // QString defaultClient() const;
    QString defaultLanguage() const;
    bool backgroundSpellChecking();
    bool skipAllUppercaseWords();
    bool skipRunTogetherWords();
    void setDocument(QTextDocument *doc);

public slots:
    void resourceChanged( int key, const QVariant & res );
    void setDefaultLanguage(const QString &lang);

private slots:
    void highlightMisspelled(const QString &word, int startPosition, bool misspelled = true);
    void dequeueDocument();
    void checkDocument(int position, int charsRemoved, int charsAdded);

    void configureSpellCheck();

private:
    Sonnet::Speller m_speller;
    QTextDocument *m_document;
    QString m_word;
    BgSpellCheck *m_bgSpellCheck;
    QQueue<QTextDocument *> m_documentsQueue;
    bool m_enableSpellCheck;
    bool m_allowSignals;
    bool m_documentIsLoading;
    QTextCharFormat m_defaultMisspelledFormat;
};

#endif
