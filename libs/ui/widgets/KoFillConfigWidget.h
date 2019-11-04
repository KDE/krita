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

#include "kritaui_export.h"

#include <QWidget>
#include <QSharedPointer>
#include <KoFlake.h>
#include <KoFlakeTypes.h>

class KoShapeFillWrapper;
class KoCanvasBase;
class KoShapeBackground;
class KoShape;

/// A widget for configuring the fill of a shape
class KRITAUI_EXPORT KoFillConfigWidget : public QWidget
{
    Q_OBJECT
    enum StyleButton {
        None = 0,
        Solid,
        Gradient,
        Pattern
    };

public:

    /**
     * @param trackShapeSelection controls if the widget connects to the canvas's selectionChanged signal.
     *                            If you decide to pass 'false', then don't forget to call
     *                            forceUpdateOnSelectionChanged() manually of every selectionChanged() and
     *                            selectionContentChanged() signals.
     */
    explicit KoFillConfigWidget(KoCanvasBase *canvas, KoFlake::FillVariant fillVariant, bool trackShapeSelection, QWidget *parent);
    ~KoFillConfigWidget() override;

    void setNoSelectionTrackingMode(bool value);

    /// Returns the list of the selected shape
    /// If you need to use only one shape, call currentShape()
    QList<KoShape*> currentShapes();

    /// returns the selected index of the fill type
    int selectedFillIndex();

    KoShapeStrokeSP createShapeStroke();

    void activate();
    void deactivate();

    void forceUpdateOnSelectionChanged();

private Q_SLOTS:
    void styleButtonPressed(int buttonId);

    void noColorSelected();
     void shapeChanged();

    /// apply color changes to the selected shape
    void colorChanged();

    /// the pattern of the fill changed, apply the changes
    void patternChanged(QSharedPointer<KoShapeBackground> background);



    void slotUpdateFillTitle();

    void slotCanvasResourceChanged(int key, const QVariant &value);

    void slotSavePredefinedGradientClicked();

    void activeGradientChanged();
    void gradientResourceChanged();

    void slotGradientTypeChanged();
    void slotGradientRepeatChanged();

    void slotProposeCurrentColorToResourceManager();
    void slotRecoverColorInResourceManager();

Q_SIGNALS:
    void sigFillChanged();

    void sigInternalRequestColorToResourceManager();
    void sigInternalRecoverColorInResourceManager();

private:
    void uploadNewGradientBackground(const QGradient *gradient);
    void setNewGradientBackgroundToShape();
    void updateGradientSaveButtonAvailability();
    void loadCurrentFillFromResourceServer();

    void updateWidgetComponentVisbility();

    /// update the widget with the KoShape background
    void updateFillIndexFromShape(KoShape *shape);

    void updateFillColorFromShape(KoShape *shape);

    class Private;
    Private * const d;
};

#endif // FILLCONFIGWIDGET_H
