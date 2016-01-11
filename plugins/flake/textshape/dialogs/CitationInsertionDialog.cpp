/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>
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
#include "CitationInsertionDialog.h"

#include <QDebug>
#include <KoInlineCite.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextDocument.h>

#include <QMessageBox>

CitationInsertionDialog::CitationInsertionDialog(KoTextEditor *editor, QWidget *parent) 
    : QDialog(parent)
    , m_blockSignals(false)
    , m_editor(editor)
{
    dialog.setupUi(this);
    connect(dialog.buttonBox, SIGNAL(accepted()), this, SLOT(insert()));
    connect(dialog.existingCites, SIGNAL(currentIndexChanged(QString)), this, SLOT(selectionChangedFromExistingCites()));

    QStringList existingCites(i18n("Select"));
    foreach (KoInlineCite *cite, KoTextDocument(m_editor->document()).inlineTextObjectManager()->citations().values()) {
        existingCites << cite->identifier();
        m_cites[cite->identifier()] = cite;
    }
    existingCites.removeDuplicates();
    dialog.existingCites->addItems(existingCites);

    show();
}

void CitationInsertionDialog::insert()
{
    if (m_cites.contains(dialog.shortName->text())) {
        if (*m_cites.value(dialog.shortName->text()) != *toCite()) {      //prompts if values are changed
            int ret = QMessageBox::warning(this, i18n("Warning"), i18n("The document already contains the bibliography entry with different data.\n"
                                           "Do you want to adjust existing entries?"), QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                Q_FOREACH (KoInlineCite *existingCite, m_cites.values(dialog.shortName->text())) {
                    *existingCite = *toCite();                       //update all cites with new values
                    existingCite->setType(KoInlineCite::ClonedCitation);    //change type to ClonedCitation
                }
                emit accept();
            } else {
                return;
            }
        }
    }
    KoInlineCite *cite = m_editor->insertCitation();
    if (dialog.shortName->text().isEmpty()) {
        const int number =
            KoTextDocument(m_editor->document()).inlineTextObjectManager()->citations().count();
        dialog.shortName->setText(i18n("Short name%1", number));

        dialog.shortName->setSelection(dialog.shortName->text().length(), 0);
    }
    *cite = *toCite();
    emit accept();
}

void CitationInsertionDialog::selectionChangedFromExistingCites()
{
    if (dialog.existingCites->currentIndex() != 0) {
        KoInlineCite *cite = m_cites[dialog.existingCites->currentText()];
        this->fillValuesFrom(cite);
    } else if (dialog.existingCites->currentIndex() == 0) {
        KoInlineCite *blankCite = new KoInlineCite(KoInlineCite::Citation);
        blankCite->setBibliographyType("Article");      //default bibliography type
        const int number =
            KoTextDocument(m_editor->document()).inlineTextObjectManager()->citations().count() + 1;
        blankCite->setIdentifier(i18n("Short name%1", number));
        fillValuesFrom(blankCite);
    }
}

KoInlineCite *CitationInsertionDialog::toCite()
{
    KoInlineCite *cite = new KoInlineCite(KoInlineCite::Citation);
    cite->setAddress(dialog.address->text());
    cite->setAnnotation(dialog.annotation->text());
    cite->setAuthor(dialog.author->text());
    cite->setBibliographyType(dialog.sourceType->currentText().remove(QLatin1Char(' ')).toLower());      //removing spaces and lowering case for exact tag attribute of bibliography-type
    cite->setBookTitle(dialog.booktitle->text());
    cite->setChapter(dialog.chapter->text());
    cite->setCustom1(dialog.ud1->text());
    cite->setCustom2(dialog.ud2->text());
    cite->setCustom3(dialog.ud3->text());
    cite->setCustom4(dialog.ud4->text());
    cite->setCustom5(dialog.ud5->text());
    cite->setEdition(dialog.edition->text());
    cite->setEditor(dialog.editor->text());
    cite->setIdentifier(dialog.shortName->text());
    cite->setInstitution(dialog.institution->text());
    cite->setISBN(dialog.isbn->text());
    cite->setISSN(dialog.issn->text());
    cite->setJournal(dialog.journal->text());
    cite->setMonth(dialog.month->text());
    cite->setNote(dialog.note->text());
    cite->setNumber(dialog.number->text());
    cite->setOrganisation(dialog.organisation->text());
    cite->setPages(dialog.pages->text());
    cite->setPublicationType(dialog.publication->text());
    cite->setPublisher(dialog.publisher->text());
    cite->setReportType(dialog.reporttype->text());
    cite->setSchool(dialog.school->text());
    cite->setSeries(dialog.series->text());
    cite->setTitle(dialog.title->text());
    cite->setURL(dialog.url->text());
    cite->setVolume(dialog.volume->text());
    cite->setYear(dialog.year->text());
    return cite;
}

void CitationInsertionDialog::fillValuesFrom(KoInlineCite *cite)
{
    dialog.address->setText(cite->address());
    dialog.annotation->setText(cite->annotation());
    dialog.author->setText(cite->author());
    dialog.sourceType->setCurrentIndex(dialog.sourceType->findText(cite->bibliographyType(), Qt::MatchFixedString));
    dialog.booktitle->setText(cite->bookTitle());
    dialog.chapter->setText(cite->chapter());
    dialog.ud1->setText(cite->custom1());
    dialog.ud2->setText(cite->custom2());
    dialog.ud3->setText(cite->custom3());
    dialog.ud4->setText(cite->custom4());
    dialog.ud5->setText(cite->custom5());
    dialog.edition->setText(cite->edition());
    dialog.editor->setText(cite->editor());
    dialog.institution->setText(cite->institution());
    dialog.shortName->setText(cite->identifier());
    dialog.isbn->setText(cite->isbn());
    dialog.issn->setText(cite->issn());
    dialog.journal->setText(cite->journal());
    dialog.month->setText(cite->month());
    dialog.note->setText(cite->note());
    dialog.number->setText(cite->number());
    dialog.organisation->setText(cite->organisations());
    dialog.pages->setText(cite->pages());
    dialog.publication->setText(cite->publicationType());
    dialog.publisher->setText(cite->publisher());
    dialog.school->setText(cite->school());
    dialog.series->setText(cite->series());
    dialog.title->setText(cite->title());
    dialog.reporttype->setText(cite->reportType());
    dialog.volume->setText(cite->volume());
    dialog.year->setText(cite->year());
    dialog.url->setText(cite->url());
}
