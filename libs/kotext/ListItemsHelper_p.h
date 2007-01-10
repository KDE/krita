/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef LISTITEMSHELPER_H
#define LISTITEMSHELPER_H

#include <QFont>
#include <QFontMetricsF>

class QTextList;

/// \internal helper class for calculating text-lists prefixes and indents
class ListItemsHelper {
public:
    ListItemsHelper(QTextList *textList, const QFont &font);
    ~ListItemsHelper() {}
    /// is meant to take a QTextList and set the indent plus the string to render on each listitem
    void recalculate();
    static bool needsRecalc(QTextList *textList);

private:
    QTextList *m_textList;
    QFontMetricsF m_fm;
    QFont m_displayFont;
};

#endif
