/*
 *  SPDX-FileCopyrightText: 2016 Alexey Kapustin <akapust1n@yandex.ru>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DLG_STROKE_SELECTION_PROPERTIES_H_
#define KIS_DLG_STROKE_SELECTION_PROPERTIES_H_

#include <KoDialog.h>
#include "KisProofingConfiguration.h"
#include <kis_types.h>
#include "KisViewManager.h"
#include "KoStrokeConfigWidget.h"
#include "ui_wdgstrokeselectionproperties.h"
#include <kis_canvas2.h>
#include <KisToolShapeUtils.h>

class KoColorSpace;
class KoColorPopupAction;

enum LinePosition
{
    Outside, Inside, Center
};

enum class DrawType
{
    CurrentBrush, Line
};

enum class ColorLineSource
{
    FGColor, BGColor, CustomColor
};

enum class ColorFillSource {
    None, PaintColor, BGColor, CustomColor, FGColor
};

struct StrokeSelectionOptions
{
    StrokeSelectionOptions ();

    int lineSize {1};
    bool brushSelected {false};
    ColorFillSource colorFillSource {ColorFillSource::None};
    ColorLineSource colorLineSource {ColorLineSource::FGColor};
    int lineDimension {1};
    KoColor color;
    KoColor fillColor;
    KoColor customColor;
    KisToolShapeUtils::FillStyle fillStyle() const;
    void lock();
};

class WdgStrokeSelection : public QWidget, public Ui::WdgStrokeSelection
{
    Q_OBJECT


public:
    WdgStrokeSelection(QWidget *parent) ;
    StrokeSelectionOptions  m_options;

    bool m_isVectorLayer;
    KisPropertiesConfigurationSP m_cfg;

    void enableControls();

Q_SIGNALS:
    void colorFillSelectorChanged();
    void colorSelectorChanged();

private Q_SLOTS:
    void on_fillBox_currentIndexChanged(int index);
    void on_typeBox_currentIndexChanged(int index);
    void on_lineColorBox_currentIndexChanged(int index);

};


class KisDlgStrokeSelection : public KoDialog
{

    Q_OBJECT

public:
    KisDlgStrokeSelection(KisImageWSP image, KisViewManager *view, bool isVectorLayer);
    ~KisDlgStrokeSelection() override;

    int getLineSize() const;
    LinePosition getLinePosition() const;
    KoColor getSelectedColor() const;
    bool isBrushSelected() const;
    KoColor getFillSelectedColor() const;
    StrokeSelectionOptions  getParams() const;

private:
    WdgStrokeSelection *m_page {0};
    KisImageWSP m_image;
    KoCanvasResourceProvider *m_resourceManager {0};
    KisDisplayColorConverter *m_converter {0};
    bool m_isVectorLayer {false};

private Q_SLOTS:
    void setColorFillButton();
    void setColorButton();
    void colorChanged(const QColor &newColor);
    void colorFillChanged(const QColor &newColor);
};



#endif // KIS_DLG_STROKE_SEL_PROPERTIES_H_
