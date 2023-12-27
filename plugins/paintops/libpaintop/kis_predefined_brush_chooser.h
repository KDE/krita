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

#include <lager/cursor.hpp>
#include <KisBrushModel.h>


class KisDoubleSliderSpinBox;
class QLabel;
class QCheckBox;

class KisDoubleSliderSpinBox;
class KisSpacingSelectionWidget;
class KisCustomBrushWidget;
class KisClipboardBrushWidget;
class KisResourceItemChooser;
class KoResource;
class KisPredefinedBrushModel;


class PAINTOP_EXPORT KisPredefinedBrushChooser : public QWidget, public Ui::WdgPredefinedBrushChooser
{

    Q_OBJECT

public:
    KisPredefinedBrushChooser(int maxBrushSize,
                              KisPredefinedBrushModel *model,
                              QWidget *parent = 0, const char *name = 0);
    ~KisPredefinedBrushChooser() override;

    void setBrush(KisBrushSP brush);
    void setImage(KisImageWSP image);

    lager::reader<bool> lightnessModeEnabled() const;

private Q_SLOTS:

    void slotResetBrush();
    void slotResetAdjustments();
    void slotOpenStampBrush();
    void slotOpenClipboardBrush();
    void slotImportNewBrushResource();
    void slotDeleteBrushResource();
    void slotNewPredefinedBrush(KoResourceSP);
    void slotBrushSelected(KoResourceSP resource);
    void slotBrushPropertyChanged(KoResourceSignature signature);

private:
    struct Private;
    const QScopedPointer<Private> m_d;

    KisBrushSP m_brush;
    KisResourceItemChooser* m_itemChooser;
    KisImageWSP m_image;
    KisCustomBrushWidget* m_stampBrushWidget;
    KisClipboardBrushWidget* m_clipboardBrushWidget;
};

#endif // KIS_PREDEFINED_BRUSH_CHOOSER_H_
