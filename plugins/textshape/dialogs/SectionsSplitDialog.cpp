/* This file is part of the KDE project
 * Copyright (C) 2015 Denis Kuplyakov <dener.kup@gmail.com>
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

#include "SectionsSplitDialog.h"

#include <KoTextDocument.h>
#include <KoSection.h>
#include <KoTextEditor.h>
#include <KoSectionUtils.h>

#include <klocalizedstring.h>

SectionsSplitDialog::SectionsSplitDialog(QWidget *parent, KoTextEditor *editor)
    : KoDialog(parent)
    , m_editor(editor)
{
    setCaption(i18n("Configure sections"));
    setButtons(KoDialog::Ok | KoDialog::Cancel);
    enableButton(KoDialog::Ok, false);
    showButtonSeparator(true);
    QWidget *form = new QWidget;
    m_widget.setupUi(form);
    setMainWidget(form);

    QList<KoSection *> secStartings = KoSectionUtils::sectionStartings(editor->blockFormat());
    QList<KoSectionEnd *> secEndings = KoSectionUtils::sectionEndings(editor->blockFormat());
    foreach (KoSection *sec, secStartings) {
        m_widget.beforeList->addItem(sec->name());
    }
    foreach (KoSectionEnd *secEnd, secEndings) {
        m_widget.afterList->addItem(secEnd->name());
    }

    connect(m_widget.beforeList, SIGNAL(itemSelectionChanged()), this, SLOT(beforeListSelection()));
    connect(m_widget.afterList, SIGNAL(itemSelectionChanged()), this, SLOT(afterListSelection()));

    connect(this, SIGNAL(okClicked()), this, SLOT(okClicked()));
}

void SectionsSplitDialog::afterListSelection()
{
    if (m_widget.afterList->selectedItems().size()) { // FIXME: more elegant way to check selection?
        enableButton(KoDialog::Ok, true);
        m_widget.beforeList->clearSelection();
    }
}

void SectionsSplitDialog::beforeListSelection()
{
    if (m_widget.beforeList->selectedItems().size()) {
        enableButton(KoDialog::Ok, true);
        m_widget.afterList->clearSelection();
    }
}

void SectionsSplitDialog::okClicked()
{
    if (m_widget.beforeList->selectedItems().size()) {
        m_editor->splitSectionsStartings(m_widget.beforeList->currentRow());
    } else {
        m_editor->splitSectionsEndings(m_widget.afterList->currentRow());
    }
}
