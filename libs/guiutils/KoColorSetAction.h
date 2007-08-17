/* This file is part of the KDE project
 * Copyright (c) 2007 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#ifndef KOCOLORSETACTION_H
#define KOCOLORSETACTION_H

#include <QAction>

#include <koguiutils_export.h>

class KoColor;

/**
 * KoColorSetAction makes use of KoColorSetWidget to show a widget for for choosing a color (colormanaged via pigment).
 * @see KoColorSetAction
 */

class KOGUIUTILS_EXPORT KoColorSetAction : public QAction
{
    Q_OBJECT

public:
    /**
      * Constructs a KoColorSetAction with the specified parent.
      *
      * @param parent The parent for this action.
      */
    KoColorSetAction(QObject *parent = 0);

    /**
     * Destructor
     */
    virtual ~KoColorSetAction();

signals:
    /**
     * Emitted every time the color changes (by calling setColor() or
     * by user interaction.
     * @param color the new color
     */
    void colorChanged(const KoColor &color);

private slots:
    void handleColorChange(const KoColor &color, bool final);
    void resizeMenu(const QSize &size);
    void showCustomColorDialog();

private:
    class KoColorSetActionPrivate;
    KoColorSetActionPrivate * const d;
};

#endif

