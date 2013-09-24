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

#ifndef FILLCONFIGWIDGET_H
#define FILLCONFIGWIDGET_H

#include "kowidgets_export.h"

#include <QWidget>
#include <QSharedPointer>

class KoCanvasBase;
class QColor;
class KoResource;
class KoShapeBackground;
class KoShape;

/// A widget for configuring the fill of a shape
class KOWIDGETS_EXPORT KoFillConfigWidget : public QWidget
{
    Q_OBJECT
    enum StyleButton {
        None,
        Solid,
        Gradient,
        Pattern
    };
public:
    explicit KoFillConfigWidget(QWidget *parent);
    ~KoFillConfigWidget();

    void setCanvas(KoCanvasBase *canvas);

    KoCanvasBase *canvas();

    /// Returns the list of the selected shape
    /// If you need to use only one shape, call currentShape()
    virtual QList<KoShape*> currentShapes();

    /// Returns the first selected shape of the ressource
    virtual KoShape *currentShape();

private slots:
    void styleButtonPressed(int buttonId);

    void noColorSelected();

    /// apply color changes to the selected shape
    void colorChanged();

    /// the gradient of the fill changed, apply the changes
    void gradientChanged(QSharedPointer<KoShapeBackground> background);

    /// the pattern of the fill changed, apply the changes
    void patternChanged(QSharedPointer<KoShapeBackground> background);

    virtual void shapeChanged();
private:
    /// update the widget with the KoShape background
    void updateWidget(KoShape *shape);

    class Private;
    Private * const d;
};

#endif // FILLCONFIGWIDGET_H
