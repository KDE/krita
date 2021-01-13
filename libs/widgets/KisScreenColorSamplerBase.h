/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
 * Copyright (C) Michael Zhou <simeirxh@gmail.com>, (C) 2018
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENCOLORSAMPLERBASE_H
#define KISSCREENCOLORSAMPLERBASE_H
#include <QWidget>
#include "kritawidgets_export.h"

class KoColor;

class KRITAWIDGETS_EXPORT KisScreenColorSamplerBase : public QWidget
{
    Q_OBJECT
public:
    KisScreenColorSamplerBase(QWidget *parent = 0) : QWidget(parent) { }
    virtual ~KisScreenColorSamplerBase() { }
    /// reloads icon(s) when theme is updated
    virtual void updateIcons() = 0;
Q_SIGNALS:
    void sigNewColorSampled(KoColor);
};

#endif // KISSCREENCOLORSAMPLERBASE_H
