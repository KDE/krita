/*
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_TOOL_COLOR_PICKER_H_
#define KIS_TOOL_COLOR_PICKER_H_

#include <QList>
#include <QTimer>

#include "KoToolFactoryBase.h"
#include "ui_wdgcolorpicker.h"
#include "kis_tool.h"
#include <flake/kis_node_shape.h>
#include <KoIcon.h>
class KoResource;
class KoColorSet;

class ColorPickerOptionsWidget : public QWidget, public Ui::ColorPickerOptionsWidget
{
    Q_OBJECT

public:
    ColorPickerOptionsWidget(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class KisToolColorPicker : public KisTool
{

    Q_OBJECT

public:
    KisToolColorPicker(KoCanvasBase* canvas);
    virtual ~KisToolColorPicker();

public:
    virtual QWidget* createOptionWidget();


    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

public slots:
    void slotSetUpdateColor(bool);
    void slotSetNormaliseValues(bool);
    void slotSetAddPalette(bool);
    void slotChangeRadius(int);
    void slotAddPalette(KoResource* resource);

private:
    void displayPickedColor();
    void pickColor(const QPointF& pos);

    bool m_toForegroundColor;
    bool m_updateColor;
    bool m_addPalette;
    bool m_normaliseValues;
    int m_radius;
    KoColor m_pickedColor;

    // used to skip some of the tablet events and don't update the colour that often
    QTimer m_colorPickerDelayTimer;

    ColorPickerOptionsWidget *m_optionsWidget;

    QList<KoColorSet*> m_palettes;
};

class KisToolColorPickerFactory : public KoToolFactoryBase
{

public:
    KisToolColorPickerFactory(const QStringList&)
            : KoToolFactoryBase("KritaSelected/KisToolColorPicker") {
        setToolTip(i18n("Select a color from the image or current layer"));
        setToolType(TOOL_TYPE_FILL);
        setPriority(15);
        setIconName(koIconNameCStr("krita_tool_color_picker"));
        setShortcut(KShortcut(Qt::Key_P));
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID_ALWAYS_ACTIVE);
    }

    virtual ~KisToolColorPickerFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolColorPicker(canvas);
    }
};


#endif // KIS_TOOL_COLOR_PICKER_H_

