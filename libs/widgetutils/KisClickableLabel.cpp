/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2023 Halla Rempt <halla@valdyas.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisClickableLabel.h"

KisClickableLabel::KisClickableLabel(QWidget* parent)
    : QLabel(parent)
{
}

KisClickableLabel::~KisClickableLabel() {}

void KisClickableLabel::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit clicked();
}

