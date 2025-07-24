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

#include "kis_tool_paint.h"
#include <flake/kis_node_shape.h>
#include <KoIcon.h>
#include <kis_icon.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kis_signal_compressor.h>
#include <kis_signal_auto_connection.h>
#include <kis_resources_snapshot.h>
#include <commands_new/KisMergeLabeledLayersCommand.h>
#include <KoCompositeOpRegistry.h>

class KisOptionCollectionWidget;
class KoGroupButton;
class KisDoubleSliderSpinBox;
class KisAngleSelector;
class KisSliderSpinBox;
class QCheckBox;
class KisColorLabelSelectorWidget;
class QPushButton;
class KisColorButton;
class QToolButton;
class KisCompositeOpComboBox;

class KisToolFill : public KisToolPaint
{
    Q_OBJECT

public:
    enum FillMode
    {
        FillMode_FillSelection,
        FillMode_FillContiguousRegion,
        FillMode_FillSimilarRegions
    };

    enum FillType
    {
        FillType_FillWithForegroundColor,
        FillType_FillWithBackgroundColor,
        FillType_FillWithPattern
    };

    enum ContiguousFillMode
    {
        ContiguousFillMode_FloodFill,
        ContiguousFillMode_BoundaryFill
    };

    enum Reference
    {
        Reference_CurrentLayer,
        Reference_AllLayers,
        Reference_ColorLabeledLayers
    };

    enum ContinuousFillMode
    {
        ContinuousFillMode_DoNotUse,
        ContinuousFillMode_FillAnyRegion,
        ContinuousFillMode_FillSimilarRegions
    };

    KisToolFill(KoCanvasBase * canvas);
    ~KisToolFill() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void beginAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void endAlternateAction(KoPointerEvent *event, AlternateAction action) override;

    QWidget* createOptionWidget() override;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;

protected:
    bool wantsAutoScroll() const override { return false; }

protected Q_SLOTS:
    void resetCursorStyle() override;
    void slotUpdateFill();

private:
    static constexpr int minimumDragDistance{4};
    static constexpr int minimumDragDistanceSquared{minimumDragDistance * minimumDragDistance};

    FillMode m_fillMode {FillMode_FillContiguousRegion};

    FillType m_fillType {FillType_FillWithForegroundColor};
    qreal m_patternScale {100.0};
    qreal m_patternRotation {0.0};
    bool m_useCustomBlendingOptions {false};
    int m_customOpacity {100};
    QString m_customCompositeOp {COMPOSITE_OVER};

    ContiguousFillMode m_contiguousFillMode {ContiguousFillMode_FloodFill};
    KoColor m_contiguousFillBoundaryColor;
    int m_threshold {8};
    int m_opacitySpread {100};
    int m_closeGap {0};
    bool m_useSelectionAsBoundary {true};

    bool m_antiAlias {true};
    int m_sizemod {0};
    int m_stopGrowingAtDarkestPixel {false};
    int m_feather {0};

    Reference m_reference {Reference_CurrentLayer};
    QList<int> m_selectedColorLabels;
    bool m_useActiveLayer {false};

    ContinuousFillMode m_continuousFillMode {ContinuousFillMode_FillAnyRegion};
    
    KisSelectionSP m_fillMask;
    QSharedPointer<KoColor> m_referenceColor;
    KisPaintDeviceSP m_referencePaintDevice;
    KisMergeLabeledLayersCommand::ReferenceNodeInfoListSP m_referenceNodeList;
    int m_previousTime;
    KisResourcesSnapshotSP m_resourcesSnapshot;
    QTransform m_transform;

    FillMode m_effectiveFillMode {FillMode_FillSelection};
    bool m_isFilling {false};
    bool m_isDragging {false};
    QPoint m_fillStartWidgetPosition;
    KisSignalCompressor m_compressorFillUpdate;
    QSharedPointer<QRect> m_dirtyRect;
    QVector<QPoint> m_seedPoints;
    KisStrokeId m_fillStrokeId;
    KisNodeSP m_previousNode {nullptr};

    KConfigGroup m_configGroup;

    KisOptionCollectionWidget *m_optionWidget {nullptr};

    KoGroupButton *m_buttonWhatToFillSelection {nullptr};
    KoGroupButton *m_buttonWhatToFillContiguous {nullptr};
    KoGroupButton *m_buttonWhatToFillSimilar {nullptr};

    KoGroupButton *m_buttonFillWithFG {nullptr};
    KoGroupButton *m_buttonFillWithBG {nullptr};
    KoGroupButton *m_buttonFillWithPattern {nullptr};
    KisDoubleSliderSpinBox *m_sliderPatternScale {nullptr};
    KisAngleSelector *m_angleSelectorPatternRotation {nullptr};
    QCheckBox *m_checkBoxCustomBlendingOptions {nullptr};
    KisSliderSpinBox *m_sliderCustomOpacity {nullptr};
    KisCompositeOpComboBox *m_comboBoxCustomCompositeOp {nullptr};

    KoGroupButton *m_buttonContiguousFillModeFloodFill {nullptr};
    KoGroupButton *m_buttonContiguousFillModeBoundaryFill {nullptr};
    KisColorButton *m_buttonContiguousFillBoundaryColor {nullptr};
    KisSliderSpinBox *m_sliderThreshold {nullptr};
    KisSliderSpinBox *m_sliderSpread {nullptr};
    KisSliderSpinBox *m_sliderCloseGap {nullptr};
    QCheckBox *m_checkBoxSelectionAsBoundary {nullptr};

    QCheckBox *m_checkBoxAntiAlias {nullptr};
    KisSliderSpinBox *m_sliderGrow {nullptr};
    QToolButton *m_buttonStopGrowingAtDarkestPixel {nullptr};
    KisSliderSpinBox *m_sliderFeather {nullptr};

    KoGroupButton *m_buttonReferenceCurrent {nullptr};
    KoGroupButton *m_buttonReferenceAll {nullptr};
    KoGroupButton *m_buttonReferenceLabeled {nullptr};
    KisColorLabelSelectorWidget *m_widgetLabels {nullptr};
    QCheckBox *m_checkBoxUseActiveLayer {nullptr};

    KoGroupButton *m_buttonDragFillDoNotUse {nullptr};
    KoGroupButton *m_buttonDragFillAny {nullptr};
    KoGroupButton *m_buttonDragFillSimilar {nullptr};

    void beginFilling(const QPoint &seedPoint);
    void addFillingOperation(const QPoint &seedPoint);
    void addFillingOperation(const QVector<QPoint> &seedPoints);
    void addUpdateOperation();
    void endFilling();

    void loadConfiguration();
    KoColor loadContiguousFillBoundaryColorFromConfig();

private Q_SLOTS:
    void slot_optionButtonStripWhatToFill_buttonToggled(KoGroupButton *button,
                                                        bool checked);
    void slot_optionButtonStripFillWith_buttonToggled(KoGroupButton *button,
                                                      bool checked);
    void slot_sliderPatternScale_valueChanged(double value);
    void slot_angleSelectorPatternRotation_angleChanged(double value);
    void slot_checkBoxUseCustomBlendingOptions_toggled(bool checked);
    void slot_sliderCustomOpacity_valueChanged(int value);
    void slot_comboBoxCustomCompositeOp_currentIndexChanged(int index);
    void slot_optionButtonStripContiguousFillMode_buttonToggled(KoGroupButton *button,
                                                                bool checked);
    void slot_buttonContiguousFillBoundaryColor_changed(const KoColor &color);
    void slot_sliderThreshold_valueChanged(int value);
    void slot_sliderSpread_valueChanged(int value);
    void slot_sliderCloseGap_valueChanged(int value);
    void slot_checkBoxSelectionAsBoundary_toggled(bool checked);
    void slot_checkBoxAntiAlias_toggled(bool checked);
    void slot_sliderGrow_valueChanged(int value);
    void slot_buttonStopGrowingAtDarkestPixel_toggled(bool enabled);
    void slot_sliderFeather_valueChanged(int value);
    void slot_optionButtonStripReference_buttonToggled(KoGroupButton *button,
                                                       bool checked);
    void slot_widgetLabels_selectionChanged();
    void slot_checkBoxUseActiveLayer_toggled(bool checked);
    void slot_optionButtonStripDragFill_buttonToggled(KoGroupButton *button,
                                                          bool checked);
    void slot_buttonReset_clicked();

    void slot_currentNodeChanged(const KisNodeSP node);
    void slot_colorSpaceChanged(const KoColorSpace *colorSpace);
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

