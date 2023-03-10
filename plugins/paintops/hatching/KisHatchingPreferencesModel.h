/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HATCHING_PREFERENCES_MODEL_H
#define KIS_HATCHING_PREFERENCES_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisHatchingPreferencesData.h"
#include "KisWidgetConnectionUtils.h"

class KisHatchingPreferencesModel : public QObject
{
    Q_OBJECT
public:
    KisHatchingPreferencesModel(lager::cursor<KisHatchingPreferencesData> optionData);

    lager::cursor<KisHatchingPreferencesData> optionData;

    LAGER_QT_CURSOR(bool, useAntialias);
    LAGER_QT_CURSOR(bool, useOpaqueBackground);
    LAGER_QT_CURSOR(bool, useSubpixelPrecision);
};

#endif // KIS_HATCHING_PREFERENCES_MODEL_H
