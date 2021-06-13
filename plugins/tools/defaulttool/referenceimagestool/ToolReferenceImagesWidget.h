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


private Q_SLOTS:
    void slotOpacitySliderChanged(qreal);
    void slotSaturationSliderChanged(qreal);
    void slotKeepAspectChanged();
    void slotSaveLocationChanged(int index);
    void slotUpdateLock(bool);
    void slotUpdateCrop(bool);

    void slotImageValuesChanged();
    void slotCropValuesChanged();

private:
    struct Private;
    const QScopedPointer<Private> d;
    void updateVisibility(bool hasSelection);
    void updateCropSliders(QList<KoShape*>);
};

#endif
