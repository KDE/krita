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
#include "ui_CitationInsertionDialog.h"
#include "TextTool.h"

#include <KAction>
#include <KDebug>
#include "KoInlineCite.h"
#include "KoTextEditor.h"
#include <KoInlineTextObjectManager.h>

#include <QWidget>
#include <QMessageBox>

CitationInsertionDialog::CitationInsertionDialog(QTextDocument *doc,QWidget *parent) :
    QDialog(parent),
    m_blockSignals(false),
    document(doc)
{
    widget.setupUi(this);
    connect(widget.buttonBox,SIGNAL(accepted()),this,SLOT(insertCitation()));
    connect(widget.buttonBox,SIGNAL(accepted()),this,SLOT(accept()));
    connect(widget.existingCites,SIGNAL(currentIndexChanged(QString)),this,SLOT(selectionChangedFromExistingCites()));
    QStringList existingCites(i18n("Select"));
    foreach (KoInlineCite *cite, KoTextDocument(document).inlineTextObjectManager()->citations()) {
        existingCites<<cite->identifier();
    }
    widget.existingCites->addItems(existingCites);
}

void CitationInsertionDialog::insertCitation()
{
    KoInlineCite *cite = KoTextEditor(document).insertCitation();

    cite->setAddress(widget.address->text());
    cite->setAnnotation(widget.annotation->text());
    cite->setAuthor(widget.author->text());
    cite->setBibliographyType(widget.sourceType->currentText());
    cite->setBookTitle(widget.booktitle->text());
    cite->setChapter(widget.chapter->text());
    cite->setCustom1(widget.ud1->text());
    cite->setCustom2(widget.ud2->text());
    cite->setCustom3(widget.ud3->text());
    cite->setCustom4(widget.ud4->text());
    cite->setCustom5(widget.ud5->text());
    cite->setEdition(widget.edition->text());
    cite->setEditor(widget.editor->text());
    cite->setIdentifier((widget.shortName->text() != "")?widget.shortName->text():(QString("Short name%1").arg(QString::number(KoTextDocument(document).inlineTextObjectManager()->citations().count()))));
    cite->setInstitution(widget.institution->text());
    cite->setISBN(widget.isbn->text());
    cite->setJournal(widget.journal->text());
    cite->setMonth(widget.month->text());
    cite->setNote(widget.note->text());
    cite->setNumber(widget.number->text());
    cite->setOrganisation(widget.organisation->text());
    cite->setPages(widget.pages->text());
    cite->setPublicationType(widget.publication->text());
    cite->setPublisher(widget.publisher->text());
    cite->setReportType(widget.reporttype->text());
    cite->setSchool(widget.school->text());
    cite->setSeries(widget.series->text());
    cite->setTitle(widget.title->text());
    cite->setURL(widget.url->text());
    cite->setVolume(widget.volume->text());
    cite->setYear(widget.year->text());
}

void CitationInsertionDialog::selectionChangedFromExistingCites()
{
    if (widget.existingCites->currentIndex() != 0) {
        KoInlineCite *cite = KoTextDocument(document).inlineTextObjectManager()->citations().at(widget.existingCites->currentIndex()-1);
        widget.address->setText(cite->address());
        widget.annotation->setText(cite->annotation());
        widget.author->setText(cite->author());
        widget.sourceType->setCurrentIndex(widget.sourceType->findText(cite->bibliographyType(),Qt::MatchFixedString));
        widget.booktitle->setText(cite->bookTitle());
        widget.chapter->setText(cite->chapter());
        widget.ud1->setText(cite->custom1());
        widget.ud2->setText(cite->custom2());
        widget.ud3->setText(cite->custom3());
        widget.ud4->setText(cite->custom4());
        widget.ud5->setText(cite->custom5());
        widget.edition->setText(cite->edition());
        widget.editor->setText(cite->editor());
        widget.institution->setText(cite->institution());
        widget.shortName->setText(cite->identifier());
        widget.isbn->setText(cite->isbn());
        widget.journal->setText(cite->journal());
        widget.month->setText(cite->month());
        widget.note->setText(cite->note());
        widget.number->setText(cite->number());
        widget.organisation->setText(cite->organisations());
        widget.pages->setText(cite->pages());
        widget.publication->setText(cite->publicationType());
        widget.publisher->setText(cite->publisher());
        widget.school->setText(cite->school());
        widget.series->setText(cite->series());
        widget.title->setText(cite->title());
        widget.reporttype->setText(cite->reportType());
        widget.volume->setText(cite->volume());
        widget.year->setText(cite->year());
        widget.url->setText(cite->url());
    }
    else if (widget.existingCites->currentIndex() == 0) {
        widget.sourceType->setCurrentIndex(0);
        widget.address->clear();
        widget.annotation->clear();
        widget.author->clear();
        widget.booktitle->clear();
        widget.chapter->clear();
        widget.edition->clear();
        widget.editor->clear();
        widget.institution->clear();
        widget.isbn->clear();
        widget.journal->clear();
        widget.month->clear();
        widget.note->clear();
        widget.number->clear();
        widget.organisation->clear();
        widget.pages->clear();
        widget.publication->clear();
        widget.publisher->clear();
        widget.school->clear();
        widget.series->clear();
        widget.shortName->setText(QString("Short name%1").arg(QString::number(KoTextDocument(document).inlineTextObjectManager()->citations().count()+1)));
        widget.title->clear();
        widget.ud1->clear();
        widget.ud2->clear();
        widget.ud3->clear();
        widget.ud4->clear();
        widget.ud5->clear();
        widget.url->clear();
        widget.volume->clear();
        widget.year->clear();
    }
}

void CitationInsertionDialog::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}


