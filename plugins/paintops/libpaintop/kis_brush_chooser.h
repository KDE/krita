/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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


class PAINTOP_EXPORT KisPredefinedBrushChooser : public QWidget, Ui::WdgPredefinedBrushChooser
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

private Q_SLOTS:

    void slotResetBrush();
    void slotSetItemSize(qreal);
    void slotSetItemRotation(qreal);
    void slotSpacingChanged();
    void slotSetItemUseColorAsMask(bool);
    void slotOpenStampBrush();
    void slotOpenClipboardBrush();
    void slotImportNewBrushResource();
    void slotDeleteBrushResource();
    void slotNewPredefinedBrush(KoResourceSP );
    void updateBrushTip(KoResourceSP , bool isChangingBrushPresets = false);

Q_SIGNALS:

    void sigBrushChanged();

private:
    KisBrushSP m_brush;
    KisResourceItemChooser* m_itemChooser;
    KisImageWSP m_image;
    KisCustomBrushWidget* m_stampBrushWidget;
    KisClipboardBrushWidget* m_clipboardBrushWidget;
};

#endif // KIS_PREDEFINED_BRUSH_CHOOSER_H_
