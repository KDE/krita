/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2004 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_AUTOGRADIENT_H_
#define _KIS_AUTOGRADIENT_H_

#include <kritaui_export.h>
#include <KoSegmentGradient.h>

#include "ui_wdgautogradient.h"

class KoGradientSegment;

class KRITAUI_EXPORT KisAutogradientEditor : public QWidget, public Ui::KisWdgAutogradient
{
    Q_OBJECT

public:
    KisAutogradientEditor(QWidget *parent);
    KisAutogradientEditor(KoSegmentGradientSP gradient, QWidget *parent, const char* name, const QString& caption, KoCanvasResourcesInterfaceSP canvasResourcesInterface);

    void activate();

    void setCompactMode(bool value);

    void setGradient(KoSegmentGradientSP gradient);

    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);
    KoCanvasResourcesInterfaceSP canvasResourcesInterface() const;

Q_SIGNALS:
    void sigGradientChanged();

private:
    void disableTransparentCheckboxes();

private:
    KoSegmentGradientSP m_autogradientResource;
    KoCanvasResourcesInterfaceSP m_canvasResourcesInterface;

private Q_SLOTS:
    void slotSelectedSegment(KoGradientSegment* segment);
    void slotChangedSegment(KoGradientSegment* segment);
    void slotChangedInterpolation(int type);
    void slotChangedColorInterpolation(int type);
    void slotChangedLeftColor(const KoColor& color);
    void slotChangedRightColor(const KoColor& color);
    void slotChangedLeftOpacity(int value);
    void slotChangedRightOpacity(int value);
    void slotChangedLeftType(QAbstractButton* button, bool checked);
    void slotChangedRightType(QAbstractButton* button, bool checked);
    void slotChangedLeftTypeTransparent(bool checked);
    void slotChangedRightTypeTransparent(bool checked);

    void slotChangedName();
    void paramChanged();
};

#endif
