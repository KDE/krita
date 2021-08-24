/*
 *  SPDX-FileCopyrightText: 2017 Eugene Ingerman
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TOOL_REFERENCE_IMAGES_WIDGET_H
#define __TOOL_REFERENCE_IMAGES_WIDGET_H

#include <QScopedPointer>
#include <QWidget>
#include <QModelIndex>

#include "kis_types.h"
#include <KoShape.h>

class KoColor;
class KoSelection;
class KisCanvasResourceProvider;
class ToolReferenceImages;

class ToolReferenceImagesWidget : public QWidget
{
    Q_OBJECT
public:
    ToolReferenceImagesWidget(ToolReferenceImages *tool, KisCanvasResourceProvider *provider = 0, QWidget *parent = 0);
    ~ToolReferenceImagesWidget() override;

    void selectionChanged(KoSelection *selection);

public Q_SLOTS:
    void slotCropRectChanged();

private Q_SLOTS:
    void slotOpacitySliderChanged(qreal);
    void slotSaturationSliderChanged(qreal);
    void slotKeepAspectChanged();
    void slotSaveLocationChanged(int index);
    void slotRotateChanged();
    void slotMirrorChanged();
    void slotPositionChanged();
    void slotZoomChanged();
    void slotPinAllChanged();
    void slotUpdateCrop(bool);
    void slotCancelCrop();

    void slotImageValuesChanged();
    void slotCropValuesChanged();

private:
    void updateVisibility(bool hasSelection);
    void updateCropSliders();

    QRectF cropRect();
    void setCropOffsetX(qreal range, qreal val);
    void setCropOffsetY(qreal range, qreal val);
    void setCropWidth(qreal range, qreal val);
    void setCropHeight(qreal range, qreal val);

    struct Private;
    const QScopedPointer<Private> d;
};

#endif
