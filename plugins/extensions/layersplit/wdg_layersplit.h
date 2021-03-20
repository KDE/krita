/*
 * SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef WDG_LAYERSPLIT_H
#define WDG_LAYERSPLIT_H

#include <QWidget>
#include "ui_wdg_layersplit.h"

class WdgLayerSplit : public QWidget, public Ui::WdgLayerSplit
{
    Q_OBJECT

public:

    WdgLayerSplit(QWidget* parent);

};


#endif // WDG_LAYERSPLIT_H
