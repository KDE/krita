/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com>
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

#include "KoFormatChangeInformation.h"

KoFormatChangeInformation::KoFormatChangeInformation(KoFormatChangeInformation::FormatChangeType formatChangeType)
{
    this->formatChangeType = formatChangeType;
}

KoFormatChangeInformation::FormatChangeType KoFormatChangeInformation::formatType()
{
    return formatChangeType;
}

KoTextStyleChangeInformation::KoTextStyleChangeInformation(KoFormatChangeInformation::FormatChangeType formatChangeType):
                              KoFormatChangeInformation(formatChangeType)
{
}

void KoTextStyleChangeInformation::setPreviousCharFormat(QTextCharFormat &previousFormat)
{
    this->previousTextCharFormat = previousFormat;
}

QTextCharFormat& KoTextStyleChangeInformation::previousCharFormat()
{
    return this->previousTextCharFormat;
}

KoParagraphStyleChangeInformation::KoParagraphStyleChangeInformation():
                                   KoTextStyleChangeInformation(KoFormatChangeInformation::eParagraphStyleChange)
{
}

void KoParagraphStyleChangeInformation::setPreviousBlockFormat(QTextBlockFormat &previousFormat)
{
    this->previousTextBlockFormat = previousFormat;
}

QTextBlockFormat& KoParagraphStyleChangeInformation::previousBlockFormat()
{
    return this->previousTextBlockFormat;
}

KoListItemNumChangeInformation::KoListItemNumChangeInformation(KoListItemNumChangeInformation::ListItemNumChangeType type):
                                                               KoFormatChangeInformation(KoFormatChangeInformation::eListItemNumberingChange),
                                                               eSubType(type)
{
}

void KoListItemNumChangeInformation::setPreviousStartNumber(int oldStartNumber)
{
    this->oldStartNumber = oldStartNumber;
}

KoListItemNumChangeInformation::ListItemNumChangeType KoListItemNumChangeInformation::listItemNumChangeType()
{
    return eSubType;
}

int KoListItemNumChangeInformation::previousStartNumber()
{
    return oldStartNumber;
}

