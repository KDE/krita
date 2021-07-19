/*
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PREDEFINED_BRUSH_CHOOSER_H_
#define KIS_PREDEFINED_BRUSH_CHOOSER_H_

#include <QLabel>
#include <kis_brush.h>
#include <QScroller>

#include "kritapaintop_export.h"
#include "ui_wdgpredefinedbrushchooser.h"




class KisDoubleSliderSpinBox;
class QLabel;
class QCheckBox;

class KisDoubleSliderSpinBox;
class KisSpacingSelectionWidget;
class KisCustomBrushWidget;
class KisClipboardBrushWidget;
class KisResourceItemChooser;
class KoResource;


class PAINTOP_EXPORT KisPredefinedBrushChooser : public QWidget, public Ui::WdgPredefinedBrushChooser
{

    Q_OBJECT

public:
    KisPredefinedBrushChooser(QWidget *parent = 0, const char *name = 0);
    ~KisPredefinedBrushChooser() override;

    KisBrushSP brush() {
        return m_brush;
    };

    void setBrush(KisBrushSP brush);
    void setBrushSize(qreal xPixels, qreal yPixels);
    void setImage(KisImageWSP image);

    void setHSLBrushTipEnabled(bool value);
    bool hslBrushTipEnabled() const;

private Q_SLOTS:

    void slotResetBrush();
    void slotResetAdjustments();
    void slotSetItemSize(qreal);
    void slotSetItemRotation(qreal);
    void slotSpacingChanged();
    void slotOpenStampBrush();
    void slotOpenClipboardBrush();
    void slotImportNewBrushResource();
    void slotDeleteBrushResource();
    void slotNewPredefinedBrush(KoResourceSP);
    void updateBrushTip(KoResourceSP, bool isChangingBrushPresets = false);
    void slotUpdateBrushModeButtonsState();
    void slotUpdateResetBrushAdjustmentsButtonState();
    void slotUpdateBrushAdjustmentsState();

    void slotWriteBrushMode();
    void slotWriteBrushAdjustments();

Q_SIGNALS:

    void sigBrushChanged();

private:
    KisBrushSP m_brush;
    KisResourceItemChooser* m_itemChooser;
    KisImageWSP m_image;
    KisCustomBrushWidget* m_stampBrushWidget;
    KisClipboardBrushWidget* m_clipboardBrushWidget;

    bool m_hslBrushTipEnabled = false;
    bool m_autoMidpointAdjustmentIsDefault = true;
};

#endif // KIS_PREDEFINED_BRUSH_CHOOSER_H_
