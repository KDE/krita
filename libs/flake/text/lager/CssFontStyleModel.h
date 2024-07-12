/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef CSSFONTSTYLEMODEL_H
#define CSSFONTSTYLEMODEL_H

#include <QObject>

#include <KoSvgText.h>
#include <lager/state.hpp>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include <kritaflake_export.h>

class KRITAFLAKE_EXPORT CssFontStyleModel: public QObject
{
    Q_OBJECT
public:
    CssFontStyleModel(lager::cursor<KoSvgText::CssFontStyleData> _data = lager::make_state(KoSvgText::CssFontStyleData(), lager::automatic_tag{}));

    // QFont::Style isn't exposed to qml.
    enum FontStyle {
        StyleNormal = QFont::StyleNormal,
        StyleItalic = QFont::StyleItalic,
        StyleOblique = QFont::StyleOblique
    };
    Q_ENUM(FontStyle)

    lager::cursor<KoSvgText::CssFontStyleData> data;

    LAGER_QT_CURSOR(FontStyle, style);
    LAGER_QT_CURSOR(qreal, value);
};

#endif // CSSFONTSTYLEMODEL_H
