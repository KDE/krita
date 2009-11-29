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

#include "AutocorrectConfigDialog.h"

#include <KLocale>
#include <KCharSelect>

#include <QHeaderView>
// #include <KoFontDia.h>

AutocorrectConfig::AutocorrectConfig(Autocorrect *autocorrect, QWidget *parent)
    : QWidget(parent),
    m_autocorrect(autocorrect)
{
    widget.setupUi(this);
    widget.upperCase->setCheckState(m_autocorrect->getUppercaseFirstCharOfSentence() ? Qt::Checked : Qt::Unchecked);
    widget.upperUpper->setCheckState(m_autocorrect->getFixTwoUppercaseChars() ? Qt::Checked : Qt::Unchecked);
    widget.autoFormatUrl->setCheckState(m_autocorrect->getAutoFormatURLs() ? Qt::Checked : Qt::Unchecked);
    widget.ignoreDoubleSpace->setCheckState(m_autocorrect->getSingleSpaces() ? Qt::Checked : Qt::Unchecked);
    widget.trimParagraphs->setCheckState(m_autocorrect->getTrimParagraphs() ? Qt::Checked : Qt::Unchecked);
    widget.autoChangeFormat->setCheckState(m_autocorrect->getAutoBoldUnderline() ? Qt::Checked : Qt::Unchecked);
    widget.autoReplaceNumber->setCheckState(m_autocorrect->getAutoFractions() ? Qt::Checked : Qt::Unchecked);
    widget.useNumberStyle->setCheckState(m_autocorrect->getAutoNumbering() ? Qt::Checked : Qt::Unchecked);
    widget.autoSuperScript->setCheckState(m_autocorrect->getSuperscriptAppendix() ? Qt::Checked : Qt::Unchecked);
    widget.capitalizeDaysName->setCheckState(m_autocorrect->getCapitalizeWeekDays() ? Qt::Checked : Qt::Unchecked);
    widget.useBulletStyle->setCheckState(m_autocorrect->getAutoFormatBulletList() ? Qt::Checked : Qt::Unchecked);
    widget.advancedAutocorrection->setCheckState(m_autocorrect->getAdvancedAutocorrect() ? Qt::Checked : Qt::Unchecked);

    /* tab 2 - Custom Quotes */
    widget.typographicDoubleQuotes->setCheckState(m_autocorrect->getReplaceDoubleQuotes() ? Qt::Checked : Qt::Unchecked);
    widget.typographicSingleQuotes->setCheckState(m_autocorrect->getReplaceSingleQuotes() ? Qt::Checked : Qt::Unchecked);
    m_singleQuotes = m_autocorrect->getTypographicSingleQuotes();
    widget.singleQuote1->setText(m_singleQuotes.begin);
    widget.singleQuote2->setText(m_singleQuotes.end);
    m_doubleQuotes = m_autocorrect->getTypographicDoubleQuotes();
    widget.doubleQuote1->setText(m_doubleQuotes.begin);
    widget.doubleQuote2->setText(m_doubleQuotes.end);
    connect(widget.typographicSingleQuotes, SIGNAL(stateChanged(int)), this, SLOT(enableSingleQuotes(int)));
    connect(widget.typographicDoubleQuotes, SIGNAL(stateChanged(int)), this, SLOT(enableDoubleQuotes(int)));
    connect(widget.singleQuote1, SIGNAL(clicked()), this, SLOT(selectSingleQuoteCharOpen()));
    connect(widget.singleQuote2, SIGNAL(clicked()), this, SLOT(selectSingleQuoteCharClose()));
    connect(widget.singleDefault, SIGNAL(clicked()), this, SLOT(setDefaultSingleQuotes()));
    connect(widget.doubleQuote1, SIGNAL(clicked()), this, SLOT(selectDoubleQuoteCharOpen()));
    connect(widget.doubleQuote2, SIGNAL(clicked()), this, SLOT(selectDoubleQuoteCharClose()));
    connect(widget.doubleDefault, SIGNAL(clicked()), this, SLOT(setDefaultDoubleQuotes()));
    enableSingleQuotes(widget.typographicSingleQuotes->checkState());
    enableDoubleQuotes(widget.typographicDoubleQuotes->checkState());

    /* tab 3 - Advanced Autocorrection */
    m_autocorrectEntries = m_autocorrect->getAutocorrectEntries();
    widget.tableWidget->setRowCount(m_autocorrectEntries.size());
    widget.tableWidget->verticalHeader()->hide();
    QHash<QString, QString>::const_iterator i = m_autocorrectEntries.constBegin();
    int j = 0;
    while (i != m_autocorrectEntries.constEnd()) {
        widget.tableWidget->setItem(j, 0, new QTableWidgetItem(i.key()));
        widget.tableWidget->setItem(j++, 1, new QTableWidgetItem(i.value()));
        ++i;
    }
    widget.tableWidget->setSortingEnabled(true);
    widget.tableWidget->sortByColumn(0, Qt::AscendingOrder);

    enableAdvAutocorrection(widget.advancedAutocorrection->checkState());
    connect(widget.advancedAutocorrection, SIGNAL(stateChanged(int)), this, SLOT(enableAdvAutocorrection(int)));
    connect(widget.autoCorrectionWithFormat, SIGNAL(stateChanged(int)), this, SLOT(enableAutocorrectFormat(int)));
    connect(widget.addButton, SIGNAL(clicked()), this, SLOT(addAutocorrectEntry()));
    connect(widget.removeButton, SIGNAL(clicked()), this, SLOT(removeAutocorrectEntry()));
    connect(widget.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(setFindReplaceText(int, int)));
    connect(widget.find, SIGNAL(textChanged(const QString &)), this, SLOT(enableAddRemoveButton()));
    connect(widget.replace, SIGNAL(textChanged(const QString &)), this, SLOT(enableAddRemoveButton()));
    connect(widget.changeFormat, SIGNAL(clicked()), this, SLOT(changeCharFormat()));

    /* tab 4 - Exceptions */
    m_upperCaseExceptions = m_autocorrect->getUpperCaseExceptions();
    m_twoUpperLetterExceptions = m_autocorrect->getTwoUpperLetterExceptions();
    widget.abbreviationList->addItems(m_upperCaseExceptions.toList());
    widget.twoUpperLetterList->addItems(m_twoUpperLetterExceptions.toList());
    widget.add1->setEnabled(false);
    widget.add2->setEnabled(false);

    connect(widget.abbreviation, SIGNAL(textChanged(const QString &)), this, SLOT(abbreviationChanged(const QString &)));
    connect(widget.twoUpperLetter, SIGNAL(textChanged(const QString &)), this, SLOT(twoUpperLetterChanged(const QString &)));
    connect(widget.add1, SIGNAL(clicked()), this, SLOT(addAbbreviationEntry()));
    connect(widget.remove1, SIGNAL(clicked()), this, SLOT(removeAbbreviationEntry()));
    connect(widget.add2, SIGNAL(clicked()), this, SLOT(addTwoUpperLetterEntry()));
    connect(widget.remove2, SIGNAL(clicked()), this, SLOT(removeTwoUpperLetterEntry()));
}

AutocorrectConfig::~AutocorrectConfig()
{
}

void AutocorrectConfig::applyConfig()
{
    m_autocorrect->setUppercaseFirstCharOfSentence(widget.upperCase->checkState() == Qt::Checked);
    m_autocorrect->setFixTwoUppercaseChars(widget.upperUpper->checkState() == Qt::Checked);
    m_autocorrect->setAutoFormatURLs(widget.autoFormatUrl->checkState() == Qt::Checked);
    m_autocorrect->setSingleSpaces(widget.ignoreDoubleSpace->checkState() == Qt::Checked);
    m_autocorrect->setTrimParagraphs(widget.trimParagraphs->checkState() == Qt::Checked);
    m_autocorrect->setAutoBoldUnderline(widget.autoChangeFormat->checkState() == Qt::Checked);
    m_autocorrect->setAutoFractions(widget.autoReplaceNumber->checkState() == Qt::Checked);
    m_autocorrect->setAutoNumbering(widget.useNumberStyle->checkState() == Qt::Checked);
    m_autocorrect->setSuperscriptAppendix(widget.autoSuperScript->checkState() == Qt::Checked);
    m_autocorrect->setCapitalizeWeekDays(widget.capitalizeDaysName->checkState() == Qt::Checked);
    m_autocorrect->setAutoFormatBulletList(widget.useBulletStyle->checkState() == Qt::Checked);
    m_autocorrect->setAdvancedAutocorrect(widget.advancedAutocorrection->checkState() == Qt::Checked);

    m_autocorrect->setAutocorrectEntries(m_autocorrectEntries);
    m_autocorrect->setUpperCaseExceptions(m_upperCaseExceptions);
    m_autocorrect->setTwoUpperLetterExceptions(m_twoUpperLetterExceptions);

    m_autocorrect->setReplaceDoubleQuotes(widget.typographicDoubleQuotes->checkState() == Qt::Checked);
    m_autocorrect->setReplaceSingleQuotes(widget.typographicSingleQuotes->checkState() == Qt::Checked);
    m_autocorrect->setTypographicSingleQuotes(m_singleQuotes);
    m_autocorrect->setTypographicDoubleQuotes(m_doubleQuotes);
}

void AutocorrectConfig::enableSingleQuotes(int state)
{
    bool enable = state == Qt::Checked;
    widget.singleQuote1->setEnabled(enable);
    widget.singleQuote2->setEnabled(enable);
    widget.singleDefault->setEnabled(enable);
}

void AutocorrectConfig::enableDoubleQuotes(int state)
{
    bool enable = state == Qt::Checked;
    widget.doubleQuote1->setEnabled(enable);
    widget.doubleQuote2->setEnabled(enable);
    widget.doubleDefault->setEnabled(enable);
}

void AutocorrectConfig::selectSingleQuoteCharOpen()
{
    CharSelectDialog *dlg = new CharSelectDialog(this);
    dlg->setCurrentChar(m_singleQuotes.begin);
    if (dlg->exec()) {
        m_singleQuotes.begin = dlg->currentChar();
        widget.singleQuote1->setText(m_singleQuotes.begin);
    }
    delete dlg;
}

void AutocorrectConfig::selectSingleQuoteCharClose()
{
    CharSelectDialog *dlg = new CharSelectDialog(this);
    dlg->setCurrentChar(m_singleQuotes.end);
    if (dlg->exec()) {
        m_singleQuotes.end = dlg->currentChar();
        widget.singleQuote2->setText(m_singleQuotes.end);
    }
    delete dlg;
}

void AutocorrectConfig::setDefaultSingleQuotes()
{
    m_singleQuotes = m_autocorrect->getTypographicDefaultSingleQuotes();
    widget.singleQuote1->setText(m_singleQuotes.begin);
    widget.singleQuote2->setText(m_singleQuotes.end);
}

void AutocorrectConfig::selectDoubleQuoteCharOpen()
{
    CharSelectDialog *dlg = new CharSelectDialog(this);
    dlg->setCurrentChar(m_doubleQuotes.begin);
    if (dlg->exec()) {
        m_doubleQuotes.begin = dlg->currentChar();
        widget.doubleQuote1->setText(m_doubleQuotes.begin);
    }
    delete dlg;
}

void AutocorrectConfig::selectDoubleQuoteCharClose()
{
    CharSelectDialog *dlg = new CharSelectDialog(this);
    dlg->setCurrentChar(m_doubleQuotes.end);
    if (dlg->exec()) {
        m_doubleQuotes.end = dlg->currentChar();
        widget.doubleQuote2->setText(m_doubleQuotes.end);
    }
    delete dlg;
}

void AutocorrectConfig::setDefaultDoubleQuotes()
{
    m_doubleQuotes = m_autocorrect->getTypographicDefaultDoubleQuotes();
    widget.doubleQuote1->setText(m_doubleQuotes.begin);
    widget.doubleQuote2->setText(m_doubleQuotes.end);
}

void AutocorrectConfig::enableAdvAutocorrection(int state)
{
    bool enable = state == Qt::Checked;
    widget.autoCorrectionWithFormat->setEnabled(enable);
    widget.findLabel->setEnabled(enable);
    widget.find->setEnabled(enable);
    widget.specialChar1->setEnabled(enable);
    widget.replaceLabel->setEnabled(enable);
    widget.replace->setEnabled(enable);
    widget.specialChar2->setEnabled(enable);
    widget.addButton->setEnabled(enable);
    widget.removeButton->setEnabled(enable);
    widget.tableWidget->setEnabled(enable);
    if (!enable) enableAutocorrectFormat(Qt::Unchecked);
}

void AutocorrectConfig::enableAutocorrectFormat(int state)
{
    bool enable = state == Qt::Checked;
    Q_UNUSED(enable);
    // widget.changeFormat->setEnabled(enable);
    // widget.clearFormat->setEnabled(enable);
}

void AutocorrectConfig::addAutocorrectEntry()
{
    int currentRow = widget.tableWidget->currentRow();
    QString find = widget.find->text();
    bool modify = false;

    // Modify actually, not add, so we want to remove item from hash
    if (currentRow != -1 && find == widget.tableWidget->item(currentRow, 0)->text()) {
        m_autocorrectEntries.remove(find);
        modify = true;
    }

    m_autocorrectEntries.insert(find, widget.replace->text());
    widget.tableWidget->setSortingEnabled(false);
    int size = widget.tableWidget->rowCount();

    if (modify) {
        widget.tableWidget->removeRow(currentRow);
        size--;
    }
    else
        widget.tableWidget->setRowCount(++size);

    QTableWidgetItem *item = new QTableWidgetItem(find);
    widget.tableWidget->setItem(size - 1, 0, item);
    widget.tableWidget->setItem(size - 1, 1, new QTableWidgetItem(widget.replace->text()));

    widget.tableWidget->setSortingEnabled(true);
    widget.tableWidget->setCurrentCell(item->row(), 0);
}

void AutocorrectConfig::removeAutocorrectEntry()
{
    widget.tableWidget->setSortingEnabled(false);
    m_autocorrectEntries.remove(widget.find->text());
    widget.tableWidget->removeRow(widget.tableWidget->currentRow());
    widget.tableWidget->setSortingEnabled(true);
}

void AutocorrectConfig::enableAddRemoveButton()
{
    QString find = widget.find->text();
    QString replace = widget.replace->text();
    int currentRow = -1;
    if (m_autocorrectEntries.contains(find)) {
        currentRow = widget.tableWidget->findItems(find, Qt::MatchCaseSensitive).first()->row();
        widget.tableWidget->setCurrentCell(currentRow, 0);
    }
    else
        currentRow = widget.tableWidget->currentRow();

    bool enable = false;
    if (currentRow == -1 || find.isEmpty() || replace.isEmpty()) // disable if no text in find/replace
        enable = !(find.isEmpty() || replace.isEmpty());
    else if (find == widget.tableWidget->item(currentRow, 0)->text()) {
        // We disable add / remove button if no text for the replacement
        enable = !widget.tableWidget->item(currentRow, 1)->text().isEmpty();
        widget.addButton->setText(i18n("&Modify"));
    }
    else if (!widget.tableWidget->item(currentRow, 1)->text().isEmpty()) {
        enable = true;
        widget.addButton->setText(i18n("&Add"));
    }

    if (currentRow != -1) {
    if (replace == widget.tableWidget->item(currentRow, 1)->text())
        widget.addButton->setEnabled(false);
    else
        widget.addButton->setEnabled(enable);
    }
    widget.removeButton->setEnabled(enable);
}

void AutocorrectConfig::setFindReplaceText(int row, int column)
{
    Q_UNUSED(column);
    widget.find->setText(widget.tableWidget->item(row, 0)->text());
    widget.replace->setText(widget.tableWidget->item(row, 1)->text());
}

void AutocorrectConfig::changeCharFormat()
{
    /* QTextCharFormat format;
    KoFontDia *dia = new KoFontDia(format, this);
    if (dia->exec())
        ; */
}

void AutocorrectConfig::abbreviationChanged(const QString &text)
{
    widget.add1->setEnabled(!text.isEmpty());
}

void AutocorrectConfig::twoUpperLetterChanged(const QString &text)
{
    widget.add2->setEnabled(!text.isEmpty());
}

void AutocorrectConfig::addAbbreviationEntry()
{
    QString text = widget.abbreviation->text();
    if (!m_upperCaseExceptions.contains(text)) {
        m_upperCaseExceptions.insert(text);
        widget.abbreviationList->addItem(text);
    }
    widget.abbreviation->clear();
}

void AutocorrectConfig::removeAbbreviationEntry()
{
    int currentRow = widget.abbreviationList->currentRow();
    QListWidgetItem *item = widget.abbreviationList->takeItem(currentRow);
    Q_ASSERT(item);
    m_upperCaseExceptions.remove(item->text());
    delete item;
}

void AutocorrectConfig::addTwoUpperLetterEntry()
{
    QString text = widget.twoUpperLetter->text();
    if (!m_twoUpperLetterExceptions.contains(text)) {
        m_twoUpperLetterExceptions.insert(text);
        widget.twoUpperLetterList->addItem(text);
    }
    widget.twoUpperLetter->clear();
}

void AutocorrectConfig::removeTwoUpperLetterEntry()
{
    int currentRow = widget.twoUpperLetterList->currentRow();
    QListWidgetItem *item = widget.twoUpperLetterList->takeItem(currentRow);
    Q_ASSERT(item);
    m_twoUpperLetterExceptions.remove(item->text());
    delete item;
}

AutocorrectConfigDialog::AutocorrectConfigDialog(Autocorrect *autocorrect, QWidget *parent)
    : KDialog(parent)
{
    ui = new AutocorrectConfig(autocorrect, this);
    connect(this, SIGNAL(okClicked()), ui, SLOT(applyConfig()));
    setMainWidget(ui);
    setCaption(i18n("Autocorrection"));
}

AutocorrectConfigDialog::~AutocorrectConfigDialog()
{
    delete ui;
}

CharSelectDialog::CharSelectDialog(QWidget *parent)
    : KDialog(parent)
{
    m_charSelect = new KCharSelect(this,
            KCharSelect::FontCombo | KCharSelect::BlockCombos | KCharSelect::CharacterTable);
    setMainWidget(m_charSelect);
    setCaption(i18n("Select Character"));
}

QChar CharSelectDialog::currentChar() const
{
    return m_charSelect->currentChar();
}

void CharSelectDialog::setCurrentChar(const QChar &c)
{
    m_charSelect->setCurrentChar(c);
}

#include "AutocorrectConfigDialog.moc"
