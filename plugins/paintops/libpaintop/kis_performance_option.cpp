/* This file is part of the KDE project
 * Copyright (C) Scott Petrovic <scottpetrovic@gmail.com>, (C) 2016
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

#include "kis_performance_option.h"
#include <klocalizedstring.h>
#include <KoColor.h>
#include <kis_types.h>
#include <ui_wdgperformanceoption.h>
#include <kis_config.h>
#include <QWidget>

KisPerformanceOption ::KisPerformanceOption (bool createConfigWidget):
        KisPaintOpOption(KisPaintOpOption::GENERAL, true),
        m_createConfigWidget(createConfigWidget)

{

    m_checkable = false;

    if (createConfigWidget) {
        QWidget* widget = new QWidget();


        Ui_wdgPerformanceOption ui;
        ui.setupUi(widget);

        setConfigurationPage(widget);
        createConfigWidget = true;

      /* // for reference later  
        connect(ui.rememberBrushSizeCheckbox, SIGNAL(clicked(bool)), this, SLOT(slotRememberBrushSize(bool)));
        connect(ui.rememberOpacityCheckbox, SIGNAL(clicked(bool)), this, SLOT(slotRememberBrushOpacity(bool)));
        connect(ui.temporarilySaveTweaksCheckbox, SIGNAL(clicked(bool)), this, SLOT(slotTemporarilySaveTweaks(bool)));
        */

    }

    setObjectName("KisPerformanceOption");

}

KisPerformanceOption::~KisPerformanceOption() {
}

/*  for reference later
void KisgGlobalSettingsOption::slotRememberBrushSize(bool rememberSize) {
    KisConfig cfg;
    cfg.setUseEraserBrushSize(rememberSize);
}
*/
