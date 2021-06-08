/*
 * SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <KisPopupSelfActivatingLineEdit.h>


KisPopupSelfActivatingLineEdit::KisPopupSelfActivatingLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
}

KisPopupSelfActivatingLineEdit::~KisPopupSelfActivatingLineEdit()
{
}


void KisPopupSelfActivatingLineEdit::focusInEvent(QFocusEvent *e)
{
    QWidget *w = window();
    if (w->windowType() == Qt::Popup) {
        w->activateWindow();
    }
    QLineEdit::focusInEvent(e);
}
