/*
 *  Copyright (c) 1999 Matthias Elter
 *  Copyright (c) 2002 Patrick Julien
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

#include "kis_tool_non_paint.h"
#include "KoToolFactory.h"
#include "ui_wdgcolorpicker.h"

class KisResource;
class KisPalette;

class ColorPickerOptionsWidget : public QWidget, public Ui::ColorPickerOptionsWidget
{
    Q_OBJECT

    public:
        ColorPickerOptionsWidget(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class KisToolColorPicker : public KisToolNonPaint {

    Q_OBJECT
    typedef KisToolNonPaint super;

public:
    KisToolColorPicker();
    virtual ~KisToolColorPicker();

public:
    virtual void setup(KActionCollection *collection);
    virtual void buttonPress(KoPointerEvent *e);
    virtual QWidget* createOptionWidget();
    virtual QWidget* optionWidget();
    virtual enumToolType toolType() { return TOOL_FILL; }
    virtual quint32 priority() { return 3; }

public slots:
    void slotSetUpdateColor(bool);
    void slotSetNormaliseValues(bool);
    void slotSetAddPalette(bool);
    void slotChangeRadius(int);
    void slotAddPalette(KisResource* resource);

private:
    void displayPickedColor();

    bool m_updateColor;
    bool m_addPalette;
    bool m_normaliseValues;
    int m_radius;
    KoColor m_pickedColor;

    ColorPickerOptionsWidget *m_optionsWidget;
    KisCanvasSubject *m_subject;
    QList<KisPalette*> m_palettes;
};

class KisToolColorPickerFactory : public KoToolFactory {

public:
    KisToolColorPickerFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolColorPicker", i18n( "Color Picker" ))
        {
            setToolTip(i18n("Select a color from the image or current layer"));
            setToolType(TOOL_TYPE_SELECTED);
            setPriority(0);
            setIcon("colorpicker");
            setShortcut( Qt::Key_P );
        }

    virtual ~KisToolColorPickerFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolColorPicker(canvas);
    }
};


#endif // KIS_TOOL_COLOR_PICKER_H_

