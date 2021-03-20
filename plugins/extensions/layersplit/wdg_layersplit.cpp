/*
 * SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "wdg_layersplit.h"

#include <QPainter>
#include <QStringList>
#include <QImage>
#include <QListWidgetItem>
#include <kis_debug.h>

#include "kis_config.h"

WdgLayerSplit::WdgLayerSplit(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
}


