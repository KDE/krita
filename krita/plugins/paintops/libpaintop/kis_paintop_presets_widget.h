/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#ifndef KIS_PAINTOP_PRESETS_WIDGET_H
#define KIS_PAINTOP_PRESETS_WIDGET_H

#include <QWidget>

class QString;
class KisPaintOpOption;
class KisPaintOpPreset;
class KisPropertiesConfiguration;

/**
 * A common widget for enabling/disabling and determining
 * the effect of tablet pressure, tilt and rotation and
 * other paintop settings.
 *
 * Every effect can be enabled/disabled through a checkbox
 * and has a popdown button that shows a settings widget.
 *
 */
class KisPaintOpPresetsWidget : public QWidget {

public:

    KisPaintOpPresetsWidget( QWidget * parent = 0 );

    ~KisPaintOpPresetsWidget();

    void addPaintOpOption( KisPaintOpOption * option );

private:

    class Private;
    Private * const m_d;

};

#endif
