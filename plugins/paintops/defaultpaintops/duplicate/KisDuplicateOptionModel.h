/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KISDUPLICATEOPTIONMODEL_H_
#define __KISDUPLICATEOPTIONMODEL_H_

#include <KisDuplicateOptionData.h>

#include <QDebug>

#include <lager/extra/qt.hpp>

class KisDuplicateOptionModel : public QObject
{
    Q_OBJECT
public:
    KisDuplicateOptionModel(lager::cursor<KisDuplicateOptionData> optionData);

    lager::cursor<KisDuplicateOptionData> optionData;

    LAGER_QT_CURSOR(bool, healing);
    LAGER_QT_CURSOR(bool, correctPerspective);
    LAGER_QT_CURSOR(bool, moveSourcePoint);
    LAGER_QT_CURSOR(bool, resetSourcePoint);
    LAGER_QT_CURSOR(bool, cloneFromProjection);
};

#endif // __KISDUPLICATEOPTIONMODEL_H_
