/*
 *  Copyright (c) 2016 Alexey Kapustin <akapust1n@yandex.ru>
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
#ifndef KIS_DLG_STROKE_SELECTION_PROPERTIES_H_
#define KIS_DLG_STROKE_SELECTION_PROPERTIES_H_

#include <KoDialog.h>
#include "KisProofingConfiguration.h"
#include <kis_types.h>
#include "KisViewManager.h"
#include "KoStrokeConfigWidget.h"
#include "ui_wdgstrokeselectionproperties.h"
#include <kis_painter.h>
#include <kis_canvas2.h>

class KoColorSpace;
class KoColorPopupAction;

enum class linePosition
{
    OUTSIDE, INSIDE, CENTER
};

enum class drawType{
    brushDraw, lineDraw
};
enum class colorFillSource {
    None, PaintColor, BGColor, CustomColor, FGColor
};

struct StrokeSelectionOptions {
    StrokeSelectionOptions ();
    int lineSize;
    bool brushSelected;
    int _colorFillSource;
    int lineColorSource;
    int lineDimension;
    KoColor color;
    KoColor fillColor;
    KoColor customColor;
    KisPainter::FillStyle fillStyle() const;
    void lock();
};

class WdgStrokeSelection : public QWidget, public Ui::WdgStrokeSelection
{
    Q_OBJECT


public:
    WdgStrokeSelection(QWidget *parent) ;
    StrokeSelectionOptions  m_options;

Q_SIGNALS:
    void colorFillSelectorChanged();
    void colorSelectorChanged();

private Q_SLOTS:
    void on_fillBox_currentIndexChanged(int index);
    void on_typeBox_currentIndexChanged(const QString &arg1);
    void on_lineColorBox_currentIndexChanged(const QString &arg1);

};


class KisDlgStrokeSelection : public KoDialog
{

    Q_OBJECT


public:
    KisDlgStrokeSelection(KisImageWSP image, KisViewManager *view, bool isVectorLayer);
    ~KisDlgStrokeSelection() override;
    int getLineSize() const;
    linePosition getLinePosition() const;
    KoColor getSelectedColor() const;
    bool isBrushSelected() const;
    KoColor getFillSelectedColor() const;
    StrokeSelectionOptions  getParams() const;
    void lockVectorLayerFunctions();
    void unlockVectorLayerFunctions();

private:
    WdgStrokeSelection * m_page;
    KisImageWSP m_image;
    KoCanvasResourceProvider *m_resourceManager;
    KisDisplayColorConverter *converter;

private Q_SLOTS:
    void setColorFillButton();
    void setColorButton();
    void colorChanged(const QColor &newColor);
    void colorFillChanged(const QColor &newColor);
};



#endif // KIS_DLG_STROKE_SEL_PROPERTIES_H_
