/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_SLIDER_H
#define KIS_SLIDER_H

#include <QAbstractSlider>

#include "krita_export.h"

/**
 * A tablet-friendly slider implementation
 *
 * Features:
 *
 * - show state as a fill
 * - move the state to the position the user clicks inside the slider
 * - change state by tilt
 * - allow manual input of numbers
 * - no drop-down
 */
class KRITAUI_EXPORT KisSlider : public QAbstractSlider
{

    Q_OBJECT

public:

    KisSlider(QWidget* parent = 0);
};

#endif // KIS_SLIDER_H
