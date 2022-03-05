/*
 *  kis_tool_fill.h - part of Krayon^Krita
 *
 *  SPDX-FileCopyrightText: 2004 Bart Coppens <kde@bartcoppens.be>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_FILL_H_
#define KIS_TOOL_FILL_H_

#include <QPoint>
#include <QList>
#include <QVector>
#include <QScopedPointer>

#include "kis_tool_paint.h"
#include <flake/kis_node_shape.h>
#include <KoIcon.h>
#include <kis_icon.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kis_signal_compressor.h>
#include <kis_signal_auto_connection.h>
#include <kis_resources_snapshot.h>

class KisOptionCollectionWidget;
class QToolButton;
class KisDoubleSliderSpinBox;
class KisAngleSelector;
class KisSliderSpinBox;
class QCheckBox;
class KisColorLabelSelectorWidget;
class QPushButton;

class KisToolFill : public KisToolPaint
{
    Q_OBJECT

public:
    enum FillMode
    {
        FillSelection,
        FillContiguousRegion
    };

    enum FillType
    {
        FillWithForegroundColor,
        FillWithBackgroundColor,
        FillWithPattern
    };

    enum Reference
    {
        CurrentLayer,
        AllLayers,
        ColorLabeledLayers
    };

    enum ContinuousFillMode
    {
        FillAnyRegion,
        FillSimilarRegions
    };

    KisToolFill(KoCanvasBase * canvas);
    ~KisToolFill() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    QWidget* createOptionWidget() override;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;

protected:
    bool wantsAutoScroll() const override { return false; }

protected Q_SLOTS:
    void resetCursorStyle() override;
    void slotUpdateContinuousFill();

private:
    static constexpr int minimumDragDistance{4};
    static constexpr int minimumDragDistanceSquared{minimumDragDistance * minimumDragDistance};

    FillMode m_fillMode;

    FillType m_fillType;
    qreal m_patternScale;
    qreal m_patternRotation;

    int m_threshold;
    int m_opacitySpread;
    bool m_useSelectionAsBoundary;

    bool m_antiAlias;
    int m_sizemod;
    int m_feather;

    Reference m_reference;
    QList<int> m_selectedColorLabels;

    ContinuousFillMode m_continuousFillMode;
    
    KisSelectionSP m_continuousFillMask;
    KoColor m_continuousFillReferenceColor;
    KisPaintDeviceSP m_referencePaintDevice;
    KisResourcesSnapshotSP m_resourcesSnapshot;
    QTransform m_transform;

    FillMode m_effectiveFillMode;
    bool m_isFilling{false};
    bool m_isDragging{false};
    QPoint m_fillStartWidgetPosition;
    KisSignalCompressor m_compressorContinuousFillUpdate;
    QVector<QPoint> m_seedPoints;
    KisStrokeId m_fillStrokeId;

    KConfigGroup m_configGroup;

    KisOptionCollectionWidget *m_optionWidget;

    QToolButton *m_buttonWhatToFillSelection;
    QToolButton *m_buttonWhatToFillContiguous;

    QToolButton *m_buttonFillWithFG;
    QToolButton *m_buttonFillWithBG;
    QToolButton *m_buttonFillWithPattern;
    KisDoubleSliderSpinBox *m_sliderPatternScale;
    KisAngleSelector *m_angleSelectorPatternRotation;

    KisSliderSpinBox *m_sliderThreshold;
    KisSliderSpinBox *m_sliderSpread;
    QCheckBox *m_checkBoxSelectionAsBoundary;

    QCheckBox *m_checkBoxAntiAlias;
    KisSliderSpinBox *m_sliderGrow;
    KisSliderSpinBox *m_sliderFeather;

    QToolButton *m_buttonReferenceCurrent;
    QToolButton *m_buttonReferenceAll;
    QToolButton *m_buttonReferenceLabeled;
    KisColorLabelSelectorWidget *m_widgetLabels;

    QToolButton *m_buttonMultipleFillAny;
    QToolButton *m_buttonMultipleFillSimilar;

    void beginFilling(const QPoint &seedPoint);
    void addFillingOperation(const QPoint &seedPoint);
    void addFillingOperation(const QVector<QPoint> &seedPoints);
    void addUpdateOperation();
    void endFilling();

    void loadConfiguration();

private Q_SLOTS:
    void slot_buttonGroupWhatToFill_idToggled(int id, bool checked);
    void slot_buttonGroupFillWith_idToggled(int id, bool checked);
    void slot_sliderPatternScale_valueChanged(double value);
    void slot_angleSelectorPatternRotation_angleChanged(double value);
    void slot_sliderThreshold_valueChanged(int value);
    void slot_sliderSpread_valueChanged(int value);
    void slot_checkBoxSelectionAsBoundary_toggled(bool checked);
    void slot_checkBoxAntiAlias_toggled(bool checked);
    void slot_sliderGrow_valueChanged(int value);
    void slot_sliderFeather_valueChanged(int value);
    void slot_buttonGroupReference_idToggled(int id, bool checked);
    void slot_widgetLabels_selectionChanged();
    void slot_buttonGroupMultipleFill_idToggled(int id, bool checked);
    void slot_buttonReset_clicked();
};


#include "KisToolPaintFactoryBase.h"

class KisToolFillFactory : public KisToolPaintFactoryBase
{

public:
    KisToolFillFactory()
            : KisToolPaintFactoryBase("KritaFill/KisToolFill") {
        setToolTip(i18n("Fill Tool"));
        setSection(ToolBoxSection::Fill);
        setPriority(0);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("krita_tool_color_fill"));
        setShortcut( QKeySequence( Qt::Key_F ) );
        setPriority(14);
    }

    ~KisToolFillFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolFill(canvas);
    }

};

#endif //__filltool_h__

