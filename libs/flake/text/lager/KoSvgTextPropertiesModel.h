/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOSVGTEXTPROPERTIESMODEL_H
#define KOSVGTEXTPROPERTIESMODEL_H

#include <QObject>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>
#include <lager/state.hpp>
#include "KoSvgTextPropertyData.h"
#include <kritaflake_export.h>

class KRITAFLAKE_EXPORT KoSvgTextPropertiesModel : public QObject
{
    Q_OBJECT
public:
    KoSvgTextPropertiesModel(lager::cursor<KoSvgTextPropertyData> _textData = (lager::make_state(KoSvgTextPropertyData())));

    lager::cursor<KoSvgTextPropertyData> textData;

    LAGER_QT_CURSOR(QVariant, writingMode);
};

#endif // KOSVGTEXTPROPERTIESMODEL_H
