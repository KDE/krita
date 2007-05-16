/* This file is part of the KDE project
 * Copyright (C)  2006 Thomas Zander <zander@kde.org>
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
#ifndef KOTEXT_H
#define KOTEXT_H

#include "kotext_export.h"

#include <QStringList>
#include <QChar>
#include <QTextCharFormat>
#include <QMetaType>

/**
 * Generic namespace of the KOffice Text library for helper methods and data.
 */
namespace KoText {
    QStringList underlineTypeList();
    QStringList underlineStyleList();

    enum Options {
        ShowTextFrames =  278622039
    };

    /// enum for a type of tabulator used
    enum TabType {
        LeftTab,        ///< A left-tab
        RightTab,       ///< A right-tab
        CenterTab,      ///< A centered-tab
        DelimitorTab    ///< A tab stopping at a certain delimiter-character
    };

    /// For paragraphs each tab definition is represented by this struct.
    struct KOTEXT_EXPORT Tab {
        Tab();
        double position;    ///< distance in point from the start of the text-shape
        TabType type;       ///< Determine which type is used.
        QChar delimiter;    ///< If type is DelimitorTab; tab until this char was found in the text.
        QTextCharFormat::UnderlineStyle leaderStyle;
        //leaderWidth       // 15.4.30 TODO
        QColor leaderColor; ///< if color is valid, then use this instead of the (current) text color
        QChar leaderText;   ///< character to print as the leader (filler of the tabbed space)
        int textStyleId;

        bool operator==(const Tab &tab) const;
    };
}

Q_DECLARE_METATYPE( KoText::Tab )

#endif
