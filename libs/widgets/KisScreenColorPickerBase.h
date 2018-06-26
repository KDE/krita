/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
 * Copyright (C) Michael Zhou <simeirxh@gmail.com>, (C) 2018
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

#ifndef KISSCREENCOLORPICKERBASE_H
#define KISSCREENCOLORPICKERBASE_H
#include <QWidget>
#include "kritawidgets_export.h"

class KoColor;

class KRITAWIDGETS_EXPORT KisScreenColorPickerBase : public QWidget
{
    Q_OBJECT
public:
    KisScreenColorPickerBase(QWidget *parent = 0) : QWidget(parent) { }
    virtual ~KisScreenColorPickerBase() { }
    /// reloads icon(s) when theme is updated
    virtual void updateIcons() = 0;
Q_SIGNALS:
    void sigNewColorPicked(KoColor);
};

#endif // KISSCREENCOLORPICKERBASE_H
