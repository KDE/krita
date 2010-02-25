/* This file is part of the KDE project
   Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "utils.h"

#include <KLocale>

#include <QtGui/QPushButton>
#include <QtGui/QFontMetrics>

KOPROPERTY_EXPORT void KoProperty::Utils::setupDotDotDotButton(QPushButton *button, const QString& toolTip, const QString& whatsThis)
{
    button->setText(i18nc("Three dots for 'Insert image from file' button", "..."));
    if (!toolTip.isEmpty())
        button->setToolTip(toolTip);
    if (!whatsThis.isEmpty())
        button->setWhatsThis(whatsThis);
    button->setFocusPolicy(Qt::NoFocus);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    const QFontMetrics fm(button->font());
    button->setFixedWidth(fm.width(button->text() + "  "));
}
