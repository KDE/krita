/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TABSIZEMODEL_H
#define TABSIZEMODEL_H

#include <QObject>
#include <KoSvgText.h>
#include <lager/state.hpp>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include <kritaflake_export.h>

class KRITAFLAKE_EXPORT TabSizeModel : public QObject
{
    Q_OBJECT
public:
    explicit TabSizeModel(lager::cursor<KoSvgText::TabSizeInfo> _data = lager::make_state(KoSvgText::TabSizeInfo(), lager::automatic_tag{}));

    enum TabSizeType {
        Absolute,
        Em,
        Ex,
        Spaces
    };
    Q_ENUM(TabSizeType)

    lager::cursor<KoSvgText::TabSizeInfo> data;

    LAGER_QT_CURSOR(qreal, value);
    LAGER_QT_CURSOR(TabSizeType, unit);

};

#endif // TABSIZEMODEL_H
