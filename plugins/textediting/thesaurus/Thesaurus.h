/* This file is part of the KDE project
 * Copyright (C) 2001,2002,2003 Daniel Naber <daniel.naber@t-online.de>
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the searchTerms of the GNU Library General Public
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

#ifndef THESAURUS_H
#define THESAURUS_H

#include <KoTextEditingPlugin.h>

#include <QUrl>

class KLineEdit;
class KPushButton;
class KHistoryComboBox;
class KProcess;
class KDialog;

class QToolButton;
class QTextDocument;
class QTabWidget;
class QListWidget;
class QListWidgetItem;
class QLabel;
class QComboBox;
class QTextBrowser;
class QTextDocument;

class Thesaurus : public KoTextEditingPlugin
{
    Q_OBJECT

public:
    Thesaurus();
    ~Thesaurus();

    void finishedWord(QTextDocument *document, int cursorPosition);
    void finishedParagraph(QTextDocument *document, int cursorPosition);
    void checkSection(QTextDocument *document, int startPosition, int endPosition);

private slots:
    void process();
    void dialogClosed();

    void slotChangeLanguage();

    void slotFindTerm();
    void slotFindTerm(const QString &term, bool add_to_history = true);
    void slotFindTermFromList(QListWidgetItem *item);
    void slotFindTermFromUrl(const QUrl &url);

    void slotGotoHistory(int index);

    void slotSetReplaceTermSyn(QListWidgetItem *item);
    void slotSetReplaceTermHypo(QListWidgetItem *item);
    void slotSetReplaceTermHyper(QListWidgetItem *item);

    void slotBack();
    void slotForward();

private:
    void findTermThesaurus(const QString &term);
    void findTermWordnet(const QString &term);
    QString formatLine(const QString &line) const;

    void setCaption();
    void updateNavButtons();

    enum Mode {grep, other};

    bool m_standAlone;
    int m_historyPos;
    int m_startPosition;
    Mode m_mode;

    KProcess *m_thesProc;
    KProcess *m_wnProc;
    KDialog *m_dialog;
    KHistoryComboBox *m_edit;
    KPushButton *m_search;
    KLineEdit *m_replaceLineEdit;

    QString m_word;
    QString m_noMatch;
    QString m_dataFile;
    QToolButton *m_back;
    QToolButton *m_forward;
    QTabWidget *m_tabWidget;
    QLabel *m_replaceLabel;
    QTextDocument *m_document;

    // Thesaurus:
    QListWidget *m_synListWidget;
    QListWidget *m_hyperListWidget;
    QListWidget *m_hypoListWidget;

    // WordNet:
    QTextBrowser *m_resultTextBrowser;
    QComboBox *m_wnComboBox;
};

#endif
