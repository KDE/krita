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

#include "SpellCheckConfigDialog.h"

#include "SpellCheck.h"

#include <KLocale>

SpellCheckConfig::SpellCheckConfig(SpellCheck *spellcheck, QWidget *parent)
    : QWidget(parent),
    m_spellcheck(spellcheck)
{
    widget.setupUi(this);

    /* widget.backend->addItems(m_spellcheck->availableBackends());
    int backendIndex = widget.backend->findText(m_spellcheck->defaultClient());
    widget.backend->setCurrentIndex(backendIndex); */

    widget.language->addItems(m_spellcheck->availableLanguages());
    int langIndex = widget.language->findText(m_spellcheck->defaultLanguage());
    widget.language->setCurrentIndex(langIndex);

    widget.bgSpellCheck->setCheckState(m_spellcheck->backgroundSpellChecking() ? Qt::Checked : Qt::Unchecked);
    widget.skipUppercase->setCheckState(m_spellcheck->skipAllUppercaseWords() ? Qt::Checked : Qt::Unchecked);
    widget.skipRunTogether->setCheckState(m_spellcheck->skipRunTogetherWords() ? Qt::Checked : Qt::Unchecked);
}

SpellCheckConfig::~SpellCheckConfig()
{
}

void SpellCheckConfig::applyConfig()
{
    // m_spellcheck->setDefaultClient(widget.backend->currentText());
    m_spellcheck->setDefaultLanguage(widget.language->currentText());
    m_spellcheck->setBackgroundSpellChecking(widget.bgSpellCheck->checkState() == Qt::Checked);
    m_spellcheck->setSkipAllUppercaseWords(widget.skipUppercase->checkState() == Qt::Checked);
    m_spellcheck->setSkipRunTogetherWords(widget.skipRunTogether->checkState() == Qt::Checked);
}

SpellCheckConfigDialog::SpellCheckConfigDialog(SpellCheck *spellcheck, QWidget *parent)
    : KDialog(parent)
{
    ui = new SpellCheckConfig(spellcheck, this);
    connect(this, SIGNAL(okClicked()), ui, SLOT(applyConfig()));
    setMainWidget(ui);
    setCaption(i18n("Configure Spell Check"));
}

SpellCheckConfigDialog::~SpellCheckConfigDialog()
{
    delete ui;
}

#include "SpellCheckConfigDialog.moc"
