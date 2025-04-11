/*
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <elter@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Emmet & Eoin O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_COLOR_SAMPLER_H_
#define KIS_TOOL_COLOR_SAMPLER_H_

#include <QTimer>
#include "KoToolFactoryBase.h"
#include "ui_wdgcolorsampler.h"
#include "kis_tool.h"
#include <kis_icon.h>
#include <KoColorSet.h>
#include <QPainter>
#include <KisAsyncColorSamplerHelper.h>

class KisTagFilterResourceProxyModel;

namespace KisToolUtils {
struct ColorSamplerConfig;
}

class ColorSamplerOptionsWidget : public QWidget, public Ui::ColorSamplerOptionsWidget
{
    Q_OBJECT

public:
    ColorSamplerOptionsWidget(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class KisToolColorSampler : public KisTool
{
    Q_OBJECT

public:
    KisToolColorSampler(KoCanvasBase *canvas);
    ~KisToolColorSampler() override;

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

        void save() const;
        void load();
    };

public:
    QWidget* createOptionWidget() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void requestUpdateOutline(const QPointF &outlineDocPoint, const KoPointerEvent *event);

    void activatePrimaryAction() override;
    void deactivatePrimaryAction() override;

    void paint(QPainter &gc, const KoViewConverter &converter) override;

protected:
    void activate(const QSet<KoShape*> &) override;
    void deactivate() override;

public Q_SLOTS:
    void slotSetUpdateColor(bool);
    void slotSetNormaliseValues(bool);
    void slotSetAddPalette(bool);
    void slotChangeRadius(int);
    void slotChangeBlend(int);
    void slotSetColorSource(int value);

private Q_SLOTS:
    void slotChangePalette(int);

    void slotColorPickerRequestedCursor(const QCursor &cursor);
    void slotColorPickerRequestedCursorReset();
    void slotColorPickerRequestedOutlineUpdate();
    void slotColorPickerSelectedColor(const KoColor &color);
    void slotColorPickerSelectionFinished(const KoColor &color);

private:
    void displaySampledColor(const KoColor &color);
    void updateOptionWidget();

    // Configuration
    QScopedPointer<KisToolUtils::ColorSamplerConfig> m_config;

    bool m_isActivated {false};
    QPointF m_outlineDocPoint;

    QRectF m_oldColorPreviewUpdateRect;

    KoColor m_sampledColor;

    KisAsyncColorSamplerHelper m_helper;

    ColorSamplerOptionsWidget *m_optionsWidget {0};
    KisTagFilterResourceProxyModel *m_tagFilterProxyModel {0};
};

class KisToolColorSamplerFactory : public KoToolFactoryBase
{
public:
    KisToolColorSamplerFactory()
            : KoToolFactoryBase("KritaSelected/KisToolColorSampler") {
        setToolTip(i18n("Color Sampler Tool"));
        setSection(ToolBoxSection::Fill);
        setPriority(2);
        setIconName(koIconNameCStr("krita_tool_color_sampler"));
        setShortcut(QKeySequence(Qt::Key_P));
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolColorSamplerFactory() override {}

    KoToolBase *createTool(KoCanvasBase *canvas) override {
        return new KisToolColorSampler(canvas);
    }
};

#endif // KIS_TOOL_COLOR_SAMPLER_H_
