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

#include <KoOdfNumberDefinition.h>

#include <klocalizedstring.h>

#include <QWidget>
#include <QDebug>

NotesConfigurationDialog::NotesConfigurationDialog(QTextDocument *doc, bool footnoteMode, QWidget *parent)
    : QDialog(parent)
    , m_document(doc)
{
    widget.setupUi(this);
    if (footnoteMode) {
        setWindowTitle(i18n("Footnote Settings"));
        footnoteSetup();
    } else {
        setWindowTitle(i18n("Endnote Settings"));
        endnoteSetup();
    }
    connect(widget.buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(apply(QAbstractButton*)));
}

void NotesConfigurationDialog::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}

void NotesConfigurationDialog::footnoteSetup()
{
    m_notesConfig = KoTextDocument(m_document).styleManager()
                    ->notesConfiguration(KoOdfNotesConfiguration::Footnote);
    if (!m_notesConfig) {
        m_notesConfig = new KoOdfNotesConfiguration(KoOdfNotesConfiguration::Footnote);
    }
    widget.prefixLineEdit->setText(m_notesConfig->numberFormat().prefix());
    widget.suffixLineEdit->setText(m_notesConfig->numberFormat().suffix());
    widget.startAtSpinBox->setValue(m_notesConfig->startValue());
    widget.endlineEdit->setText(m_notesConfig->footnoteContinuationForward());
    widget.startlineEdit->setText(m_notesConfig->footnoteContinuationBackward());

    switch (m_notesConfig->numberFormat().formatSpecification()) {
    default:
    case KoOdfNumberDefinition::Numeric:
        widget.numStyleCombo->setCurrentIndex(0);
        break;
    case KoOdfNumberDefinition::AlphabeticLowerCase:
        if (m_notesConfig->numberFormat().letterSynchronization()) {
            widget.numStyleCombo->setCurrentIndex(3);
        } else {
            widget.numStyleCombo->setCurrentIndex(1);
        }
        break;
    case KoOdfNumberDefinition::AlphabeticUpperCase:
        if (m_notesConfig->numberFormat().letterSynchronization()) {
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

    switch (m_notesConfig->numberingScheme()) {
    case KoOdfNotesConfiguration::BeginAtPage:
        widget.beginAtCombo->setCurrentIndex(0);
        break;
    case KoOdfNotesConfiguration::BeginAtChapter:
        widget.beginAtCombo->setCurrentIndex(1);
        break;
    case KoOdfNotesConfiguration::BeginAtDocument:
        widget.beginAtCombo->setCurrentIndex(2);
        break;
    }
}

void NotesConfigurationDialog::endnoteSetup()
{
    widget.continuationBox->hide();
    widget.beginAtCombo->hide();
    m_notesConfig = KoTextDocument(m_document).styleManager()
                    ->notesConfiguration(KoOdfNotesConfiguration::Endnote);
    if (!m_notesConfig) {
        m_notesConfig = new KoOdfNotesConfiguration(KoOdfNotesConfiguration::Endnote);
    }
    widget.prefixLineEdit->setText(m_notesConfig->numberFormat().prefix());
    widget.suffixLineEdit->setText(m_notesConfig->numberFormat().suffix());
    widget.startAtSpinBox->setValue(m_notesConfig->startValue());

    switch (m_notesConfig->numberFormat().formatSpecification()) {
    case KoOdfNumberDefinition::Numeric:
        widget.numStyleCombo->setCurrentIndex(0);
        break;
    case KoOdfNumberDefinition::AlphabeticLowerCase:
        if (m_notesConfig->numberFormat().letterSynchronization()) {
            widget.numStyleCombo->setCurrentIndex(3);
        } else {
            widget.numStyleCombo->setCurrentIndex(1);
        }
        break;
    case KoOdfNumberDefinition::AlphabeticUpperCase:
        if (m_notesConfig->numberFormat().letterSynchronization()) {
            widget.numStyleCombo->setCurrentIndex(4);
        } else {
            widget.numStyleCombo->setCurrentIndex(2);
        }
        break;
    default:
    case KoOdfNumberDefinition::RomanLowerCase:
        widget.numStyleCombo->setCurrentIndex(5);
        break;
    case KoOdfNumberDefinition::RomanUpperCase:
        widget.numStyleCombo->setCurrentIndex(6);
        break;
    }
}

void NotesConfigurationDialog::apply(QAbstractButton *button)
{
    if (widget.buttonBox->standardButton(button) == widget.buttonBox->Apply) {
        //set Number Format
        KoOdfNumberDefinition *numFormat = new KoOdfNumberDefinition();
        //set prefix
        numFormat->setPrefix(widget.prefixLineEdit->text());
        //set suffix
        numFormat->setSuffix(widget.suffixLineEdit->text());

        switch (widget.numStyleCombo->currentIndex()) {
        case 0:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::Numeric);
            m_notesConfig->setNumberFormat(*numFormat);
            break;
        case 1:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::AlphabeticLowerCase);
            numFormat->setLetterSynchronization(false);
            m_notesConfig->setNumberFormat(*numFormat);
            break;
        case 2:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::AlphabeticUpperCase);
            numFormat->setLetterSynchronization(false);
            m_notesConfig->setNumberFormat(*numFormat);
            break;
        case 3:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::AlphabeticLowerCase);
            numFormat->setLetterSynchronization(true);
            m_notesConfig->setNumberFormat(*numFormat);
            break;
        case 4:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::AlphabeticUpperCase);
            numFormat->setLetterSynchronization(true);
            m_notesConfig->setNumberFormat(*numFormat);
            break;
        case 5:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::RomanLowerCase);
            m_notesConfig->setNumberFormat(*numFormat);
            break;
        case 6:
            numFormat->setFormatSpecification(KoOdfNumberDefinition::RomanUpperCase);
            m_notesConfig->setNumberFormat(*numFormat);
            break;
        };
        //set Foot notes Position
        /*if(m_notesConfig->noteClass() == KoOdfNotesConfiguration::Footnote) {
            switch(widget.location_footnote->currentIndex()) {
            case 0:
                m_notesConfig->setFootnotesPosition(KoOdfNotesConfiguration::Page);
            case 1:
                m_notesConfig->setFootnotesPosition(KoOdfNotesConfiguration::Text);
            case 2:
                m_notesConfig->setFootnotesPosition(KoOdfNotesConfiguration::Section);
            case 3:
                m_notesConfig->setFootnotesPosition(KoOdfNotesConfiguration::Document);

            }
        }*/
        //set start value
        m_notesConfig->setStartValue(widget.startAtSpinBox->value());
        //set Numbering Scheme
        switch (widget.beginAtCombo->currentIndex()) {
        case 0:
            m_notesConfig->setNumberingScheme(KoOdfNotesConfiguration::BeginAtPage);
            break;
        case 1:
            m_notesConfig->setNumberingScheme(KoOdfNotesConfiguration::BeginAtChapter);
            break;
        case 2:
            m_notesConfig->setNumberingScheme(KoOdfNotesConfiguration::BeginAtDocument);
            break;
        }

        //set footnote continuation forward
        m_notesConfig->setFootnoteContinuationForward(widget.endlineEdit->text());
        //set footnote continuation backward
        m_notesConfig->setFootnoteContinuationBackward(widget.startlineEdit->text());

        //TODO
        //set citation text style

        //set citation body text style

        //set master page

        //set note paragraph style

        this->close();
    } else if (widget.buttonBox->standardButton(button) == widget.buttonBox->Discard) {
        this->close();
    }
}
