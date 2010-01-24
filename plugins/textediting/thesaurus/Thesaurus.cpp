/* This file is part of the KDE project
 * Copyright (C) 2001,2002,2003 Daniel Naber <daniel.naber@t-online.de>
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

/* Copyright (C) 2001,2002,2003 Daniel Naber <daniel.naber@t-online.de>
 * This is a thesaurus based on a subset of WordNet. It also offers an
 * almost complete WordNet 1.7 frontend (WordNet is a powerful lexical
 * database/thesaurus)
 */

/*
TODO:
-Be more verbose if the result is empty
-See the TODO's in the source below

-If no match was found, use KSpell to offer alternative spellings?
-Don't start WordNet before its tab is activated?
-Maybe remove more uncommon words. However, the "polysemy/familiarity
 count" is sometimes very low for quite common word, e.g. "sky".

-Fix "no mimesource" warning of QTextBrowser? Seems really harmless.

NOT TODO:
-Add part of speech information -- I think this would blow up the
 filesize too much
*/

#include "Thesaurus.h"

#include <QToolButton>
#include <QByteArray>
#include <QTextCursor>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QGroupBox>

#include <KTabWidget>
#include <kglobal.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <khistorycombobox.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kdebug.h>
#include <krun.h>
#include <kcombobox.h>
#include <ktextbrowser.h>
#include <kurl.h>

Thesaurus::Thesaurus()
{
    m_standAlone = false;
    m_thesProc = new KProcess;
    m_wnProc = new KProcess;

    m_dialog = new KDialog(0);
    m_dialog->setButtons(KDialog::Help | KDialog::Ok | KDialog::Cancel);
    m_dialog->setDefaultButton(KDialog::Ok);
    m_dialog->setHelp(QString(), "thesaurus");
    m_dialog->resize(600, 400);

    KConfigGroup cfg = KGlobal::config()->group("");
    m_dataFile = cfg.readEntry("datafile");
    if (m_dataFile.isEmpty())
        m_dataFile = KGlobal::dirs()->findResource("data", "koffice/thesaurus/thesaurus.txt");
    setCaption();

    m_noMatch = i18n("(No match)");

    m_historyPos = 1;
    QWidget *page = new QWidget();
    m_dialog->setMainWidget(page);
    QVBoxLayout *topLayout = new QVBoxLayout(page);
    topLayout->setMargin(KDialog::marginHint());
    topLayout->setSpacing(KDialog::spacingHint());

    QHBoxLayout *row1 = new QHBoxLayout;
    topLayout->addLayout(row1);
    m_edit = new KHistoryComboBox(page);
    QLabel editLabel(i18n("&Search for:"), page);
    editLabel.setBuddy(m_edit);

    m_search = new KPushButton(i18n("S&earch"), page);
    connect(m_search, SIGNAL(clicked()), this, SLOT(slotFindTerm()));
    row1->addWidget(&editLabel, 0);
    row1->addWidget(m_edit, 1);
    row1->addWidget(m_search, 0);
    m_back = new QToolButton(page);
    m_back->setIcon(KIcon(QString::fromLatin1("go-previous")));
    m_back->setToolTip(i18nc("@action:button Go back to the previous word in history", "Back"));
    row1->addWidget(m_back, 0);
    m_forward = new QToolButton(page);
    m_forward->setIcon(KIcon(QString::fromLatin1("go-next")));
    m_forward->setToolTip(i18nc("@action:button Go forward to the next word in history", "Forward"));
    row1->addWidget(m_forward, 0);

    KPushButton *lang = new KPushButton(i18n("Change Language..."), page);
    connect(lang, SIGNAL(clicked()), this, SLOT(slotChangeLanguage()));
    row1->addWidget(lang, 0);

    connect(m_back, SIGNAL(clicked()), this, SLOT(slotBack()));
    connect(m_forward, SIGNAL(clicked()), this, SLOT(slotForward()));

    m_tabWidget = new KTabWidget(page);
    topLayout->addWidget(m_tabWidget);

    //
    // Thesaurus Tab
    //
    QWidget *thesWidget = new QWidget(m_tabWidget);
    m_tabWidget->addTab(thesWidget, i18n("&Thesaurus"));
    QHBoxLayout *thesLayout = new QHBoxLayout;
    thesLayout->setSpacing(KDialog::spacingHint());
    thesWidget->setLayout(thesLayout);

    QGroupBox *synGroupBox = new QGroupBox(i18n("Synonyms"), thesWidget);
    QHBoxLayout *synLayout = new QHBoxLayout();
    synGroupBox->setLayout(synLayout);
    m_synListWidget = new QListWidget(synGroupBox);
    synLayout->addWidget(m_synListWidget);
    thesLayout->addWidget(synGroupBox);

    QGroupBox *hyperGroupBox = new QGroupBox(i18n("More General Words"), thesWidget);
    QHBoxLayout *hyperLayout = new QHBoxLayout();
    hyperGroupBox->setLayout(hyperLayout);
    m_hyperListWidget = new QListWidget(hyperGroupBox);
    hyperLayout->addWidget(m_hyperListWidget);
    thesLayout->addWidget(hyperGroupBox);

    QGroupBox *hypoGroupBox = new QGroupBox(i18n("More Specific Words"), thesWidget);
    QHBoxLayout *hypoLayout = new QHBoxLayout();
    hypoGroupBox->setLayout(hypoLayout);
    m_hypoListWidget = new QListWidget(hypoGroupBox);
    hypoLayout->addWidget(m_hypoListWidget);
    thesLayout->addWidget(hypoGroupBox);

    // single click -- keep display unambiguous by removing other selections:

    connect(m_synListWidget, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(slotSetReplaceTermSyn(QListWidgetItem *)));
    connect(m_hyperListWidget, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(slotSetReplaceTermHyper(QListWidgetItem *)));
    connect(m_hypoListWidget, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(slotSetReplaceTermHypo(QListWidgetItem *)));

    // double click -- set the double clicked item as the new search term

    connect(m_synListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(slotFindTermFromList(QListWidgetItem *)));
    connect(m_hyperListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(slotFindTermFromList(QListWidgetItem *)));
    connect(m_hypoListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(slotFindTermFromList(QListWidgetItem *)));

    //
    // WordNet Tab
    //

    QWidget *wnWidget = new QWidget(m_tabWidget);
    m_tabWidget->addTab(wnWidget, i18n("&Wordnet"));
    QVBoxLayout *wnLayout = new QVBoxLayout;
    wnLayout->setSpacing(KDialog::spacingHint());
    wnLayout->setMargin(KDialog::marginHint());
    wnWidget->setLayout(wnLayout);

    m_wnComboBox = new KComboBox(wnWidget);
    m_wnComboBox->setEditable(false);
    wnLayout->addWidget(m_wnComboBox);
    connect(m_wnComboBox, SIGNAL(activated(int)), this, SLOT(slotFindTerm()));

    m_resultTextBrowser = new KTextBrowser(wnWidget);
    m_resultTextBrowser->setReadOnly(true);
    connect(m_resultTextBrowser, SIGNAL(anchorClicked(const QUrl &)), this, SLOT(slotFindTermFromUrl(const QUrl &)));
    wnLayout->addWidget(m_resultTextBrowser);

    // Connect for the history box
    m_edit->setTrapReturnKey(true);        // Do not use Return as default key...
    connect(m_edit, SIGNAL(returnPressed()), this, SLOT(slotFindTerm()));
    connect(m_edit, SIGNAL(activated(int)), this, SLOT(slotGotoHistory(int)));

    QHBoxLayout *row2 = new QHBoxLayout( /*m_top_layout*/ );
    topLayout->addLayout(row2);
    m_replaceLineEdit = new KLineEdit(page);
    m_replaceLabel = new QLabel(i18n("&Replace with:"), page);
    m_replaceLabel->setBuddy(m_replaceLineEdit);
    row2->addWidget(m_replaceLabel, 0);
    row2->addWidget(m_replaceLineEdit, 1);

    // Set focus
    m_edit->setFocus();
    updateNavButtons();

    connect(m_dialog, SIGNAL(accepted()), this, SLOT(process()));
    connect(m_dialog, SIGNAL(rejected()), this, SLOT(dialogClosed()));
}

Thesaurus::~Thesaurus()
{
    KConfigGroup cfg = KGlobal::config()->group("");
    cfg.writeEntry("datafile", m_dataFile);
    // FIXME?: this hopefully fixes the problem of a wrong cursor
    // and a crash (when closing e.g. konqueror) when the thesaurus dialog
    // gets close while it was still working and showing the wait cursor
    // QApplication::restoreOverrideCursor();
    delete m_thesProc;
    delete m_wnProc;
    delete m_dialog;
}

void Thesaurus::finishedWord(QTextDocument *document, int cursorPosition)
{
    Q_UNUSED(document);
    Q_UNUSED(cursorPosition);
}

void Thesaurus::finishedParagraph(QTextDocument *document, int cursorPosition)
{
    Q_UNUSED(document);
    Q_UNUSED(cursorPosition);
}

void Thesaurus::checkSection(QTextDocument *document, int startPosition, int endPosition)
{
    if (endPosition == -1 && startPosition == -1) { // standalone
        m_standAlone = true;
        if (document)
            m_word = document->toPlainText();
        m_dialog->showButton(KDialog::Ok, false);
        m_dialog->setButtonGuiItem(KDialog::Cancel,
                KGuiItem(i18nc("@action:button Close thesaurus dialog", "&Close"), QString::fromLatin1("dialog-cancel")));
        m_replaceLineEdit->setEnabled(false);
        m_replaceLabel->setEnabled(false);
    }
    else { // called from an application, e.g. KWord
        QTextCursor cursor(document);
        cursor.setPosition(startPosition);
        cursor.setPosition(endPosition, QTextCursor::KeepAnchor);
        m_word = cursor.selectedText();
        m_document = document;
        m_startPosition = startPosition;
        m_dialog->setButtonGuiItem(KDialog::Ok,
                KGuiItem(i18n("&Replace"), QString::fromLatin1("dialog-ok")));
        slotFindTerm(m_word.trimmed());
        m_replaceLineEdit->setText(m_word.trimmed());
    }
    m_dialog->show();
}

void Thesaurus::process()
{
    QString replacement = m_replaceLineEdit->text().trimmed();
    if (replacement == m_word.trimmed()) return;

    emit startMacro(i18n("Replace Word"));
    QTextCursor cursor(m_document);
    cursor.setPosition(m_startPosition);
    cursor.setPosition(m_startPosition + m_word.trimmed().length(), QTextCursor::KeepAnchor);
    cursor.insertText(replacement);
    emit stopMacro();
}

void Thesaurus::dialogClosed()
{
    if (!m_standAlone) return;
    KConfigGroup cfg = KGlobal::config()->group("");
    cfg.writeEntry("datafile", m_dataFile);
}

void Thesaurus::slotChangeLanguage()
{
    QString filename = KFileDialog::getOpenFileName(
            KGlobal::dirs()->findResource("data", "koffice/thesaurus/thesaurus.txt"));
    if (!filename.isNull()) {
        m_dataFile = filename;
        setCaption();
    }
}

void Thesaurus::setCaption()
{
    KUrl url = KUrl();
    url.setPath(m_dataFile);
    m_dialog->setCaption(i18n("Related Words - %1" , url.fileName() ) );
}

// Enable or disable back and forward button
void Thesaurus::updateNavButtons()
{
    if (m_historyPos <= 1) // 1 = first position
        m_back->setEnabled(false);
    else
        m_back->setEnabled(true);

    if (m_historyPos >= m_edit->count())
        m_forward->setEnabled(false);
    else
        m_forward->setEnabled(true);
}

// Go to an item from the editbale combo box.
void Thesaurus::slotGotoHistory(int index)
{
    m_historyPos = m_edit->count() - index;
    slotFindTerm(m_edit->itemText(index), false);
}

// Triggered when the back button is clicked.
void Thesaurus::slotBack()
{
    m_historyPos--;
    int pos = m_edit->count() - m_historyPos;
    m_edit->setCurrentIndex(pos);
    slotFindTerm(m_edit->itemText(pos), false);
}

// Triggered when the forward button is clicked.
void Thesaurus::slotForward()
{
    m_historyPos++;
    int pos = m_edit->count() - m_historyPos;
    m_edit->setCurrentIndex(pos);
    slotFindTerm(m_edit->itemText(pos), false);
}

// Triggered when a word is selected in the list box.
void Thesaurus::slotSetReplaceTermSyn(QListWidgetItem *item)
{
    m_hyperListWidget->clearSelection();
    m_hypoListWidget->clearSelection();
    if (!item)
        return;
    m_replaceLineEdit->setText(item->text());
}

void Thesaurus::slotSetReplaceTermHyper(QListWidgetItem *item)
{
    m_synListWidget->clearSelection();
    m_hypoListWidget->clearSelection();
    if (!item)
        return;
    m_replaceLineEdit->setText(item->text());
}

void Thesaurus::slotSetReplaceTermHypo(QListWidgetItem *item)
{
    m_synListWidget->clearSelection();
    m_hyperListWidget->clearSelection();
    if (!item)
        return;
    m_replaceLineEdit->setText(item->text());
}

// Triggered when Return is pressed.
void Thesaurus::slotFindTerm()
{
    slotFindTerm(m_edit->currentText());
}

// Triggered when a list item is qreal-clicked.
void Thesaurus::slotFindTermFromList(QListWidgetItem *item)
{
    slotFindTerm(item->text());
}

// Triggered when a url is clicked
void Thesaurus::slotFindTermFromUrl(const QUrl &url)
{
    slotFindTerm(url.toString());
    m_replaceLineEdit->setText(url.toString());
}

// Triggered when a word is clicked
void Thesaurus::slotFindTerm(const QString &term, bool addToHistory)
{
    // slotSetReplaceTerm(term);
    if (term.startsWith("http://")) {
        (void) new KRun(KUrl(term),0L);
    }
    else {
        if (addToHistory && m_edit->itemText(0) != term) {
            m_edit->insertItem(0, term);
            m_historyPos = m_edit->count();
            m_edit->setCurrentIndex(0);
        }
        updateNavButtons();
        findTermThesaurus(term);
        findTermWordnet(term);
    }
}

//
// Thesaurus
//
void Thesaurus::findTermThesaurus(const QString &searchTerm)
{
    if (!QFile::exists(m_dataFile)) {
        KMessageBox::error(0, i18n("The thesaurus file '%1' was not found. "
            "Please use 'Change Language...' to select a thesaurus file.", m_dataFile));
        return;
    }

    // Find only whole words. Looks clumsy, but this way we don't have to rely on
    // features that might only be in certain versions of grep:
    QString searchTermTmp = ';' + searchTerm.trimmed() + ';';
    m_thesProc->setOutputChannelMode(KProcess::SeparateChannels);
    m_thesProc->clearProgram();
    m_thesProc->setReadChannel(QProcess::StandardOutput);
    *m_thesProc << "grep" << "-i" << searchTermTmp;
    *m_thesProc << m_dataFile;

    QStringList syn;
    QStringList hyper;
    QStringList hypo;

    m_thesProc->start();
    if (!m_thesProc->waitForFinished()) {
        KMessageBox::error(0, i18n("<b>Error:</b> Failed to execute grep."));
        return;
    }

    if (!m_thesProc->waitForReadyRead()) {
    }
    QByteArray byteArray = m_thesProc->readAllStandardOutput();
    QString stdoutString(byteArray);
    QStringList lines = stdoutString.split(QChar('\n'));

    for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it) {
        QString line = (*it);
        if (line.startsWith("  ")) {  // ignore license (two spaces)
            continue;
        }

        int sep_pos = line.indexOf("#");
        QString synPart = line.left(sep_pos);
        QString hyperPart = line.right(line.length() - sep_pos - 1);
        QStringList synTmp = synPart.split(QChar(';'));
        QStringList hyperTmp = hyperPart.split(QChar(';'));

        if (synTmp.filter(searchTerm, Qt::CaseInsensitive).size() > 0) {
            // match on the left side of the '#' -- synonyms
            for (QStringList::Iterator it2 = synTmp.begin(); it2 != synTmp.end(); ++it2) {
                // add if it's not the searchTerm itself and if it's not yet in the list
                QString term = (*it2);
                if (term.toLower() != searchTerm.toLower() && syn.contains(term) == 0 && !term.isEmpty()) {
                    syn.append(term);
                }
            }
            for (QStringList::Iterator it2 = hyperTmp.begin(); it2 != hyperTmp.end(); ++it2) {
                QString term = (*it2);
                if (term.toLower() != searchTerm.toLower() && hyper.contains(term) == 0 && !term.isEmpty()) {
                    hyper.append(term);
                }
            }
        }
        if (hyperTmp.filter(searchTerm, Qt::CaseInsensitive).size() > 0) {
            // match on the right side of the '#' -- hypernyms
            for (QStringList::Iterator it2 = synTmp.begin(); it2 != synTmp.end(); ++it2) {
                QString term = (*it2);
                if (term.toLower() != searchTerm && hypo.contains(term) == 0 && !term.isEmpty()) {
                    hypo.append(term);
                }
            }
        }
    }

    m_synListWidget->clear();
    if (syn.size() > 0) {
        syn.sort();
        m_synListWidget->addItems(syn);
        m_synListWidget->setEnabled(true);
    }
    else {
        m_synListWidget->addItem(m_noMatch);
        m_synListWidget->setEnabled(false);
    }

    m_hyperListWidget->clear();
    if (hyper.size() > 0) {
        hyper.sort();
        m_hyperListWidget->addItems(hyper);
        m_hyperListWidget->setEnabled(true);
    }
    else {
        m_hyperListWidget->addItem(m_noMatch);
        m_hyperListWidget->setEnabled(false);
    }

    m_hypoListWidget->clear();
    if (hypo.size() > 0) {
        hypo.sort();
        m_hypoListWidget->addItems(hypo);
        m_hypoListWidget->setEnabled(true);
    }
    else {
        m_hypoListWidget->addItem(m_noMatch);
        m_hypoListWidget->setEnabled(false);
    }
}

//
// WordNet
//
void Thesaurus::findTermWordnet(const QString &term)
{
    m_wnProc->setOutputChannelMode(KProcess::SeparateChannels);
    m_wnProc->clearProgram();
    *m_wnProc << "wn";
    *m_wnProc << term;

    // get all results: nouns, verbs, adjectives, adverbs (see below for order):
    if (m_wnComboBox->currentIndex() == -1 || m_wnComboBox->currentIndex() == 0) {
        *m_wnProc << "-synsn" << "-synsv" << "-synsa" << "-synsr";
        m_mode = other;
    }
    else if (m_wnComboBox->currentIndex() == 1) {
        *m_wnProc << "-simsv";
        m_mode = other;
    }
    else if (m_wnComboBox->currentIndex() == 2) {
        *m_wnProc << "-antsn" << "-antsv" << "-antsa" << "-antsr";
        m_mode = other;
    }
    else if (m_wnComboBox->currentIndex() == 3) {
        *m_wnProc << "-hypon" << "-hypov";
        m_mode = other;
    }
    else if (m_wnComboBox->currentIndex() == 4) {
        *m_wnProc << "-meron";
        m_mode = other;
    }
    else if (m_wnComboBox->currentIndex() == 5) {
        *m_wnProc << "-holon";
        m_mode = other;
    }
    else if (m_wnComboBox->currentIndex() == 6) {
        // e.g. "size -> large/small"
        *m_wnProc << "-attrn" << "-attra";
        m_mode = other;
    }
    else if (m_wnComboBox->currentIndex() == 7) {
        // e.g. "kill -> die"
        *m_wnProc << "-causv";
        m_mode = other;
    }
    else if (m_wnComboBox->currentIndex() == 8) {
        // e.g. "walk -> step"
        *m_wnProc << "-entav";
        m_mode = other;
    }
    else if (m_wnComboBox->currentIndex() == 9) {
        *m_wnProc << "-famln" << "-famlv" << "-famla" << "-famlr";
        m_mode = other;
    }
    else if (m_wnComboBox->currentIndex() == 10) {
        *m_wnProc << "-framv";
        m_mode = other;
    }
    else if (m_wnComboBox->currentIndex() == 11) {
        *m_wnProc << "-grepn" << "-grepv" << "-grepa" << "-grepr";
        m_mode = grep;
    }
    else if (m_wnComboBox->currentIndex() == 12) {
        *m_wnProc << "-over";
        m_mode = other;
    }
    *m_wnProc << "-g";    // "Display gloss"

    int current = m_wnComboBox->currentIndex() != -1 ? m_wnComboBox->currentIndex() : 0;    // remember current position
    m_wnComboBox->clear();

    // warning: order matters!
    // 0:
    m_wnComboBox->insertItem(-1, i18n("Synonyms/Hypernyms - Ordered by Frequency"));
    m_wnComboBox->insertItem(-1, i18n("Synonyms - Ordered by Similarity of Meaning (verbs only)"));
    m_wnComboBox->insertItem(-1, i18n("Antonyms - Words with Opposite Meanings"));
    m_wnComboBox->insertItem(-1, i18n("Hyponyms - ... is a (kind of) %1", m_edit->currentText()));
    m_wnComboBox->insertItem(-1, i18n("Meronyms - %1 has a ...", m_edit->currentText()));
    // 5:
    m_wnComboBox->insertItem(-1, i18n("Holonyms - ... has a %1", m_edit->currentText()));
    m_wnComboBox->insertItem(-1, i18n("Attributes"));
    m_wnComboBox->insertItem(-1, i18n("Cause To (for some verbs only)"));
    m_wnComboBox->insertItem(-1, i18n("Verb Entailment (for some verbs only)"));
    m_wnComboBox->insertItem(-1, i18n("Familiarity & Polysemy Count"));
    // 10:
    m_wnComboBox->insertItem(-1, i18n("Verb Frames (examples of use)"));
    m_wnComboBox->insertItem(-1, i18n("List of Compound Words"));
    m_wnComboBox->insertItem(-1, i18n("Overview of Senses"));

    /** NOT todo:
      * -Hypernym tree: layout is difficult, you can get the same information
      *  by following links
      * -Coordinate terms (sisters): just go to synset and then use hyponyms
      * -Has Part Meronyms, Has Substance Meronyms, Has Member Meronyms,
      *  Member of Holonyms, Substance of Holonyms, Part of Holonyms:
      *  these are just subsets of Meronyms/Holonyms
      * -hmern, hholn: these are just compact versions, you can get the
      *  same information by following some links
      */

    /** TODO?:
      * -pert (e.g. nuclear -> nuclues, but "=>" are nested, difficult to display)
      * -nomn(n|v), e.g. deny -> denial, but this doesn't seem to work?
      */

    m_wnComboBox->setCurrentIndex(current);    // reset previous position

    if (m_wnProc->state() == QProcess::Running) {
        // should never happen
        kDebug(31000) <<"Warning: findTerm(): process is already running?!";
        return;
    }

    m_wnProc->start();
    if (!m_wnProc->waitForFinished()) {
        m_resultTextBrowser->setHtml(i18n("<b>Error:</b> Failed to execute WordNet program 'wn'. "
                    "WordNet has to be installed on your computer if you want to use it, "
                    "and 'wn' has to be in your PATH. "
                    "You can get WordNet at <a href=\"http://www.cogsci.princeton.edu/~wn/\">"
                    "http://www.cogsci.princeton.edu/~wn/</a>. Note that WordNet only supports "
                    "the English language."));
        m_wnComboBox->setEnabled(false);
        return;
    }

    QString stdoutString = m_wnProc->readAllStandardOutput();
    if (stdoutString.isEmpty()) {
        m_resultTextBrowser->setHtml(i18n("No match for '%1'.", m_edit->currentText()));
    }
    else {
        // render in a table, each line one row:
        QStringList lines = stdoutString.split(QChar('\n'), QString::SkipEmptyParts);
        QString result = "<qt><table>\n";
        // TODO in Qt > 3.01: try without the following line (it's necessary to ensure the
        // first column is really always quite small):
        result += "<tr><td width=\"10%\"></td><td width=\"90%\"></td></tr>\n";
        uint ct = 0;
        for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it) {
            QString l = (*it);
            // Remove some lines:
            QRegExp re("^\\d+( of \\d+)? senses? of \\w+");
            if (re.indexIn(l) != -1) {
                continue;
            }
            // Escape XML:
            l = l.replace('&', "&amp;");
            l = l.replace('<', "&lt;");
            l = l.replace('>', "&gt;");
            // TODO?:
            // move "=>" in own column?
            l = formatLine(l);
            // Table layout:
            result += "<tr>";
            if (l.startsWith(' ')) {
                result += "\t<td width=\"15\"></td>";
                l = l.trimmed();
                result += "\t<td>" + l + "</td>";
            }
            else {
                l = l.trimmed();
                result += "<td colspan=\"2\">" + l + "</td>";
            }
            result += "</tr>\n";
            ct++;
        }
        result += "\n</table></qt>\n";
        m_resultTextBrowser->setHtml(result);
        //kDebug() << result;
    }
}

//
// Tools
//

// Format lines using Qt's simple richtext.
QString Thesaurus::formatLine(const QString &line) const
{
    QString l = line;
    if (l == "--------------")
        return QString("<hr>");

    QRegExp re;

    re.setPattern("^(\\d+\\.)(.*)$");
    if (re.indexIn(l) != -1) {
        l = "<b>" +re.cap(1)+ "</b>" +re.cap(2);
        return l;
    }

    re.setPattern("^.* of (noun|verb|adj|adv) .*");
    if (re.indexIn(l) != -1) {
        l = "<font size=\"5\">" +re.cap()+ "</font>\n\n";
        return l;
    }

    if (m_mode == grep) {
        l = l.trimmed();
        return QString("<a href=\"" +l+ "\">" +l+ "</a>");
    }

    re.setPattern("^(Sense \\d+)");
    if (re.indexIn(l) != -1) {
        l = "<b>" +re.cap()+ "</b>\n";
        return l;
    }

    re.setPattern("(.*)(Also See-&gt;)(.*)");
    // Example: first sense of verb "keep"
    if (re.indexIn(l) != -1) {
        l = re.cap(1);
        l += re.cap(2);
        QStringList links = re.cap(3).split(QChar(';'), QString::SkipEmptyParts);
        for (QStringList::Iterator it = links.begin(); it != links.end(); ++it) {
            QString link = (*it);
            if (it != links.begin()) {
                l += ", ";
            }
            link = link.trimmed();
            link = link.remove(QRegExp("#\\d+"));
            l += "<a href=\"" + link + "\">" + link + "</a>";
        }
        l.prepend (' ');        // indent in table
    }

    re.setPattern("(.*)(=&gt;|HAS \\w+:|PART OF:)(.*) --");
    re.setMinimal(true);    // non-greedy
    if (re.indexIn(l) != -1) {
        int dash_pos = l.indexOf("--");
        QString line_end = l.mid(dash_pos+2, l.length()-dash_pos);
        l = re.cap(1);
        l += re.cap(2) + ' ';
        QStringList links = re.cap(3).split(QChar(','), QString::SkipEmptyParts);
        for (QStringList::Iterator it = links.begin(); it != links.end(); ++it) {
            QString link = (*it);
            if (it != links.begin()) {
                l += ", ";
            }
            link = link.trimmed();
            l += "<a href=\"" +link+ "\">" +link+ "</a>";
        }
        l += "<font color=\"#777777\">" +line_end+ "</font>";
        l.prepend(' ');        // indent in table
        return l;
    }
    re.setMinimal(false);    // greedy again

    return l;
}

#include <Thesaurus.moc>
