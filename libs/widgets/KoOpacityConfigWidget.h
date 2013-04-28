/* This file is part of the KDE project
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

#ifndef OPACITYCONFIGWIDGET_H
#define OPACITYCONFIGWIDGET_H

#include "kowidgets_export.h"

#include <QWidget>

class KoCanvasBase;
class KoColor;
class KoResource;
class KoShapeBackground;
class KoShape;

/// A widget for configuring the opacity of a shape
class KOWIDGETS_EXPORT KoOpacityConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KoOpacityConfigWidget(QWidget *parent);
    ~KoOpacityConfigWidget();

    void setCanvas(KoCanvasBase *canvas);

private slots:
    /// update the opacity of the shape
    void updateOpacity(qreal opacity);

    void shapeChanged();

private:
    class Private;
    Private * const d;
};

#endif // OPACITYCONFIGWIDGET_H
