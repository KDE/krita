/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
 * Copyright (C) Michael Zhou <simeirxh@gmail.com>, (C) 2018
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
