/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2004 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_SEGMENT_GRADIENT_EDITOR_H_
#define _KIS_SEGMENT_GRADIENT_EDITOR_H_

#include <kritaui_export.h>
#include <KoSegmentGradient.h>

#include "ui_wdgsegmentgradienteditor.h"

class KoGradientSegment;

class KRITAUI_EXPORT KisSegmentGradientEditor : public QWidget, public Ui::KisWdgSegmentGradientEditor
{
    Q_OBJECT

public:
    KisSegmentGradientEditor(QWidget *parent);
    KisSegmentGradientEditor(KoSegmentGradientSP gradient, QWidget *parent, const char* name, const QString& caption, KoCanvasResourcesInterfaceSP canvasResourcesInterface);

    void setGradient(KoSegmentGradientSP gradient);
    void gradient() const;

    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);
    KoCanvasResourcesInterfaceSP canvasResourcesInterface() const;

    void setCompactMode(bool value);

Q_SIGNALS:
    void sigGradientChanged();

private:
    KoSegmentGradientSP m_gradient;
    KoCanvasResourcesInterfaceSP m_canvasResourcesInterface;
    QAction *m_editHandleAction;
    QAction *m_deleteSegmentAction;
    QAction *m_flipSegmentAction;
    QAction *m_splitSegmentAction;
    QAction *m_duplicateSegmentAction;
    QAction *m_deleteStopAction;
    QAction *m_centerStopAction;
    QAction *m_centerMidPointAction;

private Q_SLOTS:
    void on_nameedit_editingFinished();
    void on_gradientSlider_selectedHandleChanged();
    
    void on_segmentLeftEditor_positionChanged(double position);
    void on_segmentLeftEditor_colorTypeChanged(KisGradientWidgetsUtils::ColorType type);
    void on_segmentLeftEditor_transparentToggled(bool checked);
    void on_segmentLeftEditor_colorChanged(KoColor color);
    void on_segmentLeftEditor_opacityChanged(double opacity);
    void on_segmentRightEditor_positionChanged(double position);
    void on_segmentRightEditor_colorTypeChanged(KisGradientWidgetsUtils::ColorType type);
    void on_segmentRightEditor_transparentToggled(bool checked);
    void on_segmentRightEditor_colorChanged(KoColor color);
    void on_segmentRightEditor_opacityChanged(double opacity);
    void on_segmentInterpolationTypeComboBox_activated(int value);
    void on_segmentColorInterpolationTypeComboBox_activated(int value);

    void on_stopPositionSlider_valueChanged(double position);
    void on_stopLeftEditor_colorTypeChanged(KisGradientWidgetsUtils::ColorType type);
    void on_stopLeftEditor_transparentToggled(bool checked);
    void on_stopLeftEditor_colorChanged(KoColor color);
    void on_stopLeftEditor_opacityChanged(double opacity);
    void on_stopRightEditor_colorTypeChanged(KisGradientWidgetsUtils::ColorType type);
    void on_stopRightEditor_transparentToggled(bool checked);
    void on_stopRightEditor_colorChanged(KoColor color);
    void on_stopRightEditor_opacityChanged(double opacity);
    void on_constrainStopButton_keepAspectRatioChanged(bool keep);

    void on_midPointPositionSlider_valueChanged(double opacity);

    void editSelectedHandle();
};

#endif
