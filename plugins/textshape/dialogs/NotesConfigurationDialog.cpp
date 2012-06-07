/* This file is part of the KDE project
 * Copyright (C) 2011 Brijesh Patel <brijesh3105@gmail.com>
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
#include "NotesConfigurationDialog.h"
#include "KoTextDocument.h"
#include "KoStyleManager.h"

#include <KAction>

#include <QWidget>
#include <QDebug>

NotesConfigurationDialog::NotesConfigurationDialog(QTextDocument *doc, QWidget *parent)
        : QDialog(parent),
          document(doc)
{
    widget.setupUi(this);
    footnoteSetup(true);
    connect(widget.footnote,SIGNAL(toggled(bool)),this,SLOT(footnoteSetup(bool)));
    connect(widget.endnote,SIGNAL(toggled(bool)),this,SLOT(endnoteSetup(bool)));
    connect(widget.buttonBox,SIGNAL(clicked(QAbstractButton*)),this,SLOT(apply(QAbstractButton*)));
}

KoOdfNotesConfiguration *NotesConfigurationDialog::notesConfiguration() const
{
    return notesConfig;
}

void NotesConfigurationDialog::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}

void NotesConfigurationDialog::footnoteSetup(bool on)
{
    KoOdfNotesConfiguration *notesConfig = KoTextDocument(document).
            styleManager()->notesConfiguration(KoOdfNotesConfiguration::Footnote);
    if(on && notesConfig) {
        widget.prefixLineEdit->setText(notesConfig->numberFormat().prefix());
        widget.suffixLineEdit->setText(notesConfig->numberFormat().suffix());
        widget.startAtSpinBox->setValue(notesConfig->startValue());
        widget.endlineEdit->setText(notesConfig->footnoteContinuationForward());
        widget.startlineEdit->setText(notesConfig->footnoteContinuationBackward());

        switch(notesConfig->numberFormat().formatSpecification()) {
        case KoOdfNumberDefinition::Numeric:
            widget.numStyleCombo->setCurrentIndex(0);
            break;
        case KoOdfNumberDefinition::AlphabeticLowerCase:
            if (notesConfig->numberFormat().letterSynchronization()) {
                widget.numStyleCombo->setCurrentIndex(3);
            } else {
                widget.numStyleCombo->setCurrentIndex(1);
            }
            break;
        case KoOdfNumberDefinition::AlphabeticUpperCase:
            if (notesConfig->numberFormat().letterSynchronization()) {
                widget.numStyleCombo->setCurrentIndex(4);
            } else {
                widget.numStyleCombo->setCurrentIndex(2);
            }
            break;
        case KoOdfNumberDefinition::RomanLowerCase:
            widget.numStyleCombo->setCurrentIndex(5);
            break;
        case KoOdfNumberDefinition::RomanUpperCase:
            widget.numStyleCombo->setCurrentIndex(6);
            break;
        }

        switch(notesConfig->numberingScheme()) {
        case KoOdfNotesConfiguration::BeginAtDocument:
            widget.beginAtCombo->setCurrentIndex(0);
            break;
        case KoOdfNotesConfiguration::BeginAtPage:
            widget.beginAtCombo->setCurrentIndex(1);
            break;
        case KoOdfNotesConfiguration::BeginAtChapter:
            widget.beginAtCombo->setCurrentIndex(2);
            break;
        }
    }
}

void NotesConfigurationDialog::endnoteSetup(bool on)
{
    widget.dockWidget_5->setHidden(on);
    widget.beginAtCombo->setDisabled(on);
    KoOdfNotesConfiguration *notesConfig = KoTextDocument(document).
            styleManager()->notesConfiguration(KoOdfNotesConfiguration::Endnote);
    if(on && notesConfig) {
        widget.prefixLineEdit->setText(notesConfig->numberFormat().prefix());
        widget.suffixLineEdit->setText(notesConfig->numberFormat().suffix());
        widget.startAtSpinBox->setValue(notesConfig->startValue());

        switch(notesConfig->numberFormat().formatSpecification()) {
        case KoOdfNumberDefinition::Numeric:
            widget.numStyleCombo->setCurrentIndex(0);
            break;
        case KoOdfNumberDefinition::AlphabeticLowerCase:
            if (notesConfig->numberFormat().letterSynchronization()) {
                widget.numStyleCombo->setCurrentIndex(3);
            } else {
                widget.numStyleCombo->setCurrentIndex(1);
            }
            break;
        case KoOdfNumberDefinition::AlphabeticUpperCase:
            if (notesConfig->numberFormat().letterSynchronization()) {
                widget.numStyleCombo->setCurrentIndex(4);
            } else {
                widget.numStyleCombo->setCurrentIndex(2);
            }
            break;
        case KoOdfNumberDefinition::RomanLowerCase:
            widget.numStyleCombo->setCurrentIndex(5);
            break;
        case KoOdfNumberDefinition::RomanUpperCase:
            widget.numStyleCombo->setCurrentIndex(6);
            break;
        }
    }

}

void NotesConfigurationDialog::apply(QAbstractButton *button)
{
    if (widget.buttonBox->standardButton(button) == widget.buttonBox->Apply) {
        notesConfig = new KoOdfNotesConfiguration();
        //set Note Class
        if (widget.footnote->isChecked()) {
            notesConfig->setNoteClass(KoOdfNotesConfiguration::Footnote);
        } else {
            notesConfig->setNoteClass(KoOdfNotesConfiguration::Endnote);
        }
        //set Number Format
        KoOdfNumberDefinition *numFormat = new KoOdfNumberDefinition();
        //set prefix
        numFormat->setPrefix(widget.prefixLineEdit->text());
        //set suffix
        numFormat->setSuffix(widget.suffixLineEdit->text());

        switch(widget.numStyleCombo->currentIndex()) {
        case 0:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::Numeric);
            notesConfig->setNumberFormat(*numFormat);
            break;
        case 1:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::AlphabeticLowerCase);
            numFormat->setLetterSynchronization(false);
            notesConfig->setNumberFormat(*numFormat);
            break;
        case 2:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::AlphabeticUpperCase);
            numFormat->setLetterSynchronization(false);
            notesConfig->setNumberFormat(*numFormat);
            break;
        case 3:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::AlphabeticLowerCase);
            numFormat->setLetterSynchronization(true);
            notesConfig->setNumberFormat(*numFormat);
            break;
        case 4:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::AlphabeticUpperCase);
            numFormat->setLetterSynchronization(true);
            notesConfig->setNumberFormat(*numFormat);
            break;
        case 5:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::RomanLowerCase);
            notesConfig->setNumberFormat(*numFormat);
            break;
        case 6:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::RomanUpperCase);
            notesConfig->setNumberFormat(*numFormat);
            break;
        };
        //set Foot notes Position
        /*if(notesConfig->noteClass() == KoOdfNotesConfiguration::Footnote) {
            switch(widget.location_footnote->currentIndex()) {
            case 0:
                notesConfig->setFootnotesPosition(KoOdfNotesConfiguration::Page);
            case 1:
                notesConfig->setFootnotesPosition(KoOdfNotesConfiguration::Text);
            case 2:
                notesConfig->setFootnotesPosition(KoOdfNotesConfiguration::Section);
            case 3:
                notesConfig->setFootnotesPosition(KoOdfNotesConfiguration::Document);

            }
        }*/
        //set start value
        notesConfig->setStartValue(widget.startAtSpinBox->value());
        //set Numbering Scheme
        switch(widget.beginAtCombo->currentIndex()) {
        case 0:
            notesConfig->setNumberingScheme(KoOdfNotesConfiguration::BeginAtDocument);
            break;
        case 1:
            notesConfig->setNumberingScheme(KoOdfNotesConfiguration::BeginAtPage);
            break;
        case 2:
            notesConfig->setNumberingScheme(KoOdfNotesConfiguration::BeginAtChapter);
            break;
        }

        //set footnote continuation forward
        notesConfig->setFootnoteContinuationForward(widget.endlineEdit->text());
        //set footnote continuation backward
        notesConfig->setFootnoteContinuationBackward(widget.startlineEdit->text());

        //TODO
        //set citation text style

        //set citation body text style

        //set master page

        //set note paragraph style

        //KoTextDocument(document).setNotesConfiguration(notesConfig);
        KoTextDocument(document).styleManager()->setNotesConfiguration(notesConfig);
        this->close();
    }
    else if (widget.buttonBox->standardButton(button) == widget.buttonBox->Discard) {
        this->close();
    }
}

#include <NotesConfigurationDialog.moc>
