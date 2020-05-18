/*
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2018 Emmet & Eoin O'Neill <emmetoneill.pdx@gmail.com>
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

#include <QTimer>
#include "KoToolFactoryBase.h"
#include "ui_wdgcolorpicker.h"
#include "kis_tool.h"
#include <kis_icon.h>

#include <KoResourceServerObserver.h>

class KoResource;
class KoColorSet;

namespace KisToolUtils {
struct ColorPickerConfig;
}

class ColorPickerOptionsWidget : public QWidget, public Ui::ColorPickerOptionsWidget
{
    Q_OBJECT

public:
    ColorPickerOptionsWidget(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class KisToolColorPicker : public KisTool,public KoResourceServerObserver<KoColorSet>
{
    Q_OBJECT
    Q_PROPERTY(bool toForeground READ toForeground WRITE setToForeground NOTIFY toForegroundChanged)

public:
    KisToolColorPicker(KoCanvasBase *canvas);
    ~KisToolColorPicker() override;

public:
    struct Configuration {
        Configuration();

        bool toForegroundColor;
        bool updateColor;
        bool addPalette;
        bool normaliseValues;
        bool sampleMerged;
        int radius;
        int blend;

        void save(ToolActivation activation) const;
        void load(ToolActivation activation);
    };

public:
    QWidget* createOptionWidget() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    void paint(QPainter &gc, const KoViewConverter &converter) override;

    bool toForeground() const;

public: //KoResourceServerObserver
    void unsetResourceServer() override;
    void resourceAdded(KoColorSet* resource) override;
    void removingResource(KoColorSet* resource) override;
    void resourceChanged(KoColorSet* resource) override;
    void syncTaggedResourceView() override;
    void syncTagAddition(const QString& tag) override;
    void syncTagRemoval(const QString& tag) override;

Q_SIGNALS:
    void toForegroundChanged();

protected:
    void activate(ToolActivation activation, const QSet<KoShape*> &) override;
    void deactivate() override;

public Q_SLOTS:
    void setToForeground(bool newValue);
    void slotSetUpdateColor(bool);
    void slotSetNormaliseValues(bool);
    void slotSetAddPalette(bool);
    void slotChangeRadius(int);
    void slotChangeBlend(int);
    void slotSetColorSource(int value);

private:
    void displayPickedColor();
    bool pickColor(const QPointF& pos);
    void updateOptionWidget();
    void updateCmbPalette();
    // Configuration
    QScopedPointer<KisToolUtils::ColorPickerConfig> m_config;

    ToolActivation m_toolActivationSource;
    bool m_isActivated;

    KoColor m_pickedColor;

    // Used to skip some tablet events and update color less often
    QTimer m_colorPickerDelayTimer;

    ColorPickerOptionsWidget *m_optionsWidget;

    QList<KoColorSet*> m_palettes;
};

class KisToolColorPickerFactory : public KoToolFactoryBase
{
public:
    KisToolColorPickerFactory()
            : KoToolFactoryBase("KritaSelected/KisToolColorPicker") {
        setToolTip(i18n("Color Selector Tool"));
        setSection(TOOL_TYPE_FILL);
        setPriority(2);
        setIconName(koIconNameCStr("krita_tool_color_picker"));
        setShortcut(QKeySequence(Qt::Key_P));
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolColorPickerFactory() override {}

    KoToolBase *createTool(KoCanvasBase *canvas) override {
        return new KisToolColorPicker(canvas);
    }
};

#endif // KIS_TOOL_COLOR_PICKER_H_
