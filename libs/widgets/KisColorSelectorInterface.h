/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISCOLORSELECTORINTERFACE_H
#define KISCOLORSELECTORINTERFACE_H

#include "kritawidgets_export.h"

#include <QWidget>
#include <KoColor.h>

class KoColorDisplayRendererInterface;

class KRITAWIDGETS_EXPORT KisColorSelectorInterface : public QWidget {
    Q_OBJECT
public:
    KisColorSelectorInterface(QWidget *parent = 0)
        : QWidget(parent)
    {}
    ~KisColorSelectorInterface() override {}
    virtual void setConfig(bool forceCircular, bool forceSelfUpdate)
    {
        Q_UNUSED(forceCircular);
        Q_UNUSED(forceSelfUpdate);
    }
    virtual void setDisplayRenderer (const KoColorDisplayRendererInterface *displayRenderer)
    {
        Q_UNUSED(displayRenderer);
    }

    virtual KoColor getCurrentColor() const = 0;

Q_SIGNALS:
    void sigNewColor(const KoColor &c);

public Q_SLOTS:
    virtual void slotSetColor(const KoColor &c) = 0;
};

#endif // KISCOLORSELECTORINTERFACE_H
