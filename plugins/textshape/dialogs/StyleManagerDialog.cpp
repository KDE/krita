/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "StyleManagerDialog.h"
#include "StyleManager.h"

#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>
#include <QMessageBox>

StyleManagerDialog::StyleManagerDialog(QWidget *parent)
    : KoDialog(parent)
{
    setButtons(Ok | Cancel | Apply);
    m_styleManagerWidget = new StyleManager(this);
    setMainWidget(m_styleManagerWidget);
    setWindowTitle(i18n("Style Manager"));

    connect(this, SIGNAL(applyClicked()), this, SLOT(applyClicked()));
}

StyleManagerDialog::~StyleManagerDialog()
{
}

void StyleManagerDialog::applyClicked()
{
    if (m_styleManagerWidget->checkUniqueStyleName()) {
        m_styleManagerWidget->save();
    }
}

void StyleManagerDialog::accept()
{
    if (!m_styleManagerWidget->checkUniqueStyleName()) {
        return;
    }
    m_styleManagerWidget->save();
    KoDialog::accept();
    deleteLater();
}

void StyleManagerDialog::reject()
{
    if (m_styleManagerWidget->unappliedStyleChanges()) {
        int ans = QMessageBox::warning(this, i18n("Save Changes"), i18n("You have changes that are not applied. "
                                       "What do you want to do with those changes?"), QMessageBox::Apply, QMessageBox::Discard, QMessageBox::Cancel);
        switch (ans) {
        case QMessageBox::Apply :
            if (m_styleManagerWidget->checkUniqueStyleName()) {
                m_styleManagerWidget->save();
                break;
            }
            return;
        case QMessageBox::Discard :
            break;
        case QMessageBox::Cancel :
            return;
        }
    }
    KoDialog::reject();
    deleteLater();
}

void StyleManagerDialog::closeEvent(QCloseEvent *e)
{
    e->ignore();
    reject();
}

void StyleManagerDialog::setStyleManager(KoStyleManager *sm)
{
    m_styleManagerWidget->setStyleManager(sm);
}

void StyleManagerDialog::setUnit(const KoUnit &unit)
{
    m_styleManagerWidget->setUnit(unit);
}

void StyleManagerDialog::setCharacterStyle(KoCharacterStyle *style, bool canDelete)
{
    m_styleManagerWidget->setCharacterStyle(style, canDelete);
}

void StyleManagerDialog::setParagraphStyle(KoParagraphStyle *style)
{
    m_styleManagerWidget->setParagraphStyle(style);
}
