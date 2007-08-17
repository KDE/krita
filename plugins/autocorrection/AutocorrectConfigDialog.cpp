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

#include "Autocorrect.h"

#include <KLocale>

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

    widget.typographicDoubleQuotes->setCheckState(m_autocorrect->getReplaceDoubleQuotes() ? Qt::Checked : Qt::Unchecked);
    widget.typographicSimpleQuotes->setCheckState(m_autocorrect->getReplaceSingleQuotes() ? Qt::Checked : Qt::Unchecked);
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

    m_autocorrect->setReplaceDoubleQuotes(widget.typographicDoubleQuotes->checkState() == Qt::Checked);
    m_autocorrect->setReplaceSingleQuotes(widget.typographicSimpleQuotes->checkState() == Qt::Checked);
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

#include "AutocorrectConfigDialog.moc"
