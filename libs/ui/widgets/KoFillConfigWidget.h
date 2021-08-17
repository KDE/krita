/* This file is part of the KDE project
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * SPDX-FileCopyrightText: 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef FILLCONFIGWIDGET_H
#define FILLCONFIGWIDGET_H

#include "kritaui_export.h"

#include <QWidget>
#include <QSharedPointer>
#include <KoFlake.h>
#include <KoFlakeTypes.h>
#include <SvgMeshGradient.h>

class KoShapeFillWrapper;
class KoCanvasBase;
class KoShapeBackground;
class KoShape;
class KoColor;

/// A widget for configuring the fill of a shape
class KRITAUI_EXPORT KoFillConfigWidget : public QWidget
{
    Q_OBJECT
    enum StyleButton {
        None = 0,
        Solid,
        Gradient,
        Pattern,
        MeshGradient
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

    void setSelectedMeshGradientHandle(const SvgMeshPosition &position);

private Q_SLOTS:
    void styleButtonPressed(int buttonId);

    void noColorSelected();
    void shapeChanged();

    /// apply color changes to the selected shape
    void colorChanged(std::pair<QColor, KoFlake::FillVariant> resource);

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

    /// this won't preserve the rows and columns
    void slotMeshGradientChanged();
    void slotMeshGradientShadingChanged(int index);
    void slotMeshHandleColorChanged(const KoColor &c);

Q_SIGNALS:
    void sigFillChanged();

    void sigInternalRequestColorToResourceManager();
    void sigInternalRecoverColorInResourceManager();
    void sigMeshGradientResetted();

private:
    void updateGradientUi(const QGradient *gradient);
    void setNewGradientBackgroundToShape();
    void updateGradientSaveButtonAvailability();
    void loadCurrentFillFromResourceServer();

    /// sets the active gradient either from the shape (if present) or creates a new one
    void createNewMeshGradientBackground();
    void createNewDefaultMeshGradientBackground();
    void setNewMeshGradientBackgroundToShape();
    void updateMeshGradientUI();

    void updateWidgetComponentVisbility();

    /// updates the UI based on KoFlake::FillType it gets from the shape.
    void updateUiFromFillType(KoShape *shape);

    class Private;
    Private * const d;
};

#endif // FILLCONFIGWIDGET_H
