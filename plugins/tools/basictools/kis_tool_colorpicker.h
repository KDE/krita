/*
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2018 Emmet & Eoin O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_COLOR_PICKER_H_
#define KIS_TOOL_COLOR_PICKER_H_

#include <QTimer>
#include "KoToolFactoryBase.h"
#include "ui_wdgcolorpicker.h"
#include "kis_tool.h"
#include <kis_icon.h>
#include <KoColorSet.h>

class KisResourceModel;

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

class KisToolColorPicker : public KisTool
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

    // Configuration
    QScopedPointer<KisToolUtils::ColorPickerConfig> m_config;

    ToolActivation m_toolActivationSource {ToolActivation::DefaultActivation};
    bool m_isActivated {false};

    KoColor m_pickedColor;

    // Used to skip some tablet events and update color less often
    QTimer m_colorPickerDelayTimer;

    ColorPickerOptionsWidget *m_optionsWidget {0};
    KisResourceModel *m_resourceModel {0};
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
