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

#include "ListsSpinBox.h"

#include "ListItemsHelper.h"

ListsSpinBox::ListsSpinBox(QWidget *parent)
    : QSpinBox(parent)
    , m_type(KoListStyle::DecimalItem)
    , m_letterSynchronization(false)
{
}

void ListsSpinBox::setCounterType(KoListStyle::Style type)
{
    m_type = type;
    update();
}

int ListsSpinBox::valueFromText(const QString &text) const
{
    Q_UNUSED(text);
    return 0;
}

QString ListsSpinBox::textFromValue(int value) const
{
    switch (m_type) {
    case KoListStyle::DecimalItem:
        return QString::number(value);
    case KoListStyle::AlphaLowerItem:
        return Lists::intToAlpha(value, Lists::Lowercase, m_letterSynchronization);
    case KoListStyle::UpperAlphaItem:
        return Lists::intToAlpha(value, Lists::Uppercase, m_letterSynchronization);
    case KoListStyle::RomanLowerItem:
        return Lists::intToRoman(value);
    case KoListStyle::UpperRomanItem:
        return Lists::intToRoman(value).toUpper();
    case KoListStyle::Bengali:
    case KoListStyle::Gujarati:
    case KoListStyle::Gurumukhi:
    case KoListStyle::Kannada:
    case KoListStyle::Malayalam:
    case KoListStyle::Oriya:
    case KoListStyle::Tamil:
    case KoListStyle::Telugu:
    case KoListStyle::Tibetan:
    case KoListStyle::Thai:
        return Lists::intToScript(value, m_type);
    case KoListStyle::Abjad:
    case KoListStyle::ArabicAlphabet:
    case KoListStyle::AbjadMinor:
        return Lists::intToScriptList(value, m_type);
    default:  // others we ignore.
        return "X";
    }
}
