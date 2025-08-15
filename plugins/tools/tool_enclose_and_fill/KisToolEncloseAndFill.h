/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTOOLENCLOSEANDFILL_H
#define KISTOOLENCLOSEANDFILL_H

#include <QPoint>
#include <QList>

#include <kis_icon.h>
#include <kis_tool_shape.h>
#include <flake/kis_node_shape.h>
#include <KoIcon.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <KisEncloseAndFillPainter.h>
#include <commands_new/KisMergeLabeledLayersCommand.h>
#include <KoCompositeOpRegistry.h>

#include "subtools/KisDynamicDelegatedTool.h"

class KisOptionCollectionWidget;
class KisColorButton;
class KoGroupButton;
class KisDoubleSliderSpinBox;
class KisAngleSelector;
class KisSliderSpinBox;
class QCheckBox;
class KisColorLabelSelectorWidget;
class QPushButton;
class QToolButton;
class QComboBox;
class KisCompositeOpComboBox;

class KisToolEncloseAndFill : public KisDynamicDelegatedTool<KisToolShape>
{
    Q_OBJECT

public:
    enum EnclosingMethod
    {
        Rectangle,
        Ellipse,
        Path,
        Lasso,
        Brush
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

    using RegionSelectionMethod = KisEncloseAndFillPainter::RegionSelectionMethod;

    KisToolEncloseAndFill(KoCanvasBase * canvas);
    ~KisToolEncloseAndFill() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void activateAlternateAction(AlternateAction action) override;
    void deactivateAlternateAction(AlternateAction action) override;
    void beginAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void endAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void beginAlternateDoubleClickAction(KoPointerEvent *event, AlternateAction action) override;

    int flags() const override;
    QWidget* createOptionWidget() override;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;

private:
    EnclosingMethod m_enclosingMethod {Lasso};

    RegionSelectionMethod m_regionSelectionMethod {defaultRegionSelectionMethod()};
    KoColor m_regionSelectionColor;
    bool m_regionSelectionInvert {false};
    bool m_regionSelectionIncludeContourRegions {false};

    FillType m_fillType {FillWithForegroundColor};
    qreal m_patternScale {100.0};
    qreal m_patternRotation {0.0};
    bool m_useCustomBlendingOptions {false};
    int m_customOpacity {100};
    QString m_customCompositeOp {COMPOSITE_OVER};

    int m_fillThreshold {8};
    int m_fillOpacitySpread {100};
    int m_closeGap {0};
    bool m_useSelectionAsBoundary {true};

    bool m_antiAlias {false};
    int m_expand {0};
    int m_stopGrowingAtDarkestPixel {false};
    int m_feather {0};
    bool m_useActiveLayer {false};

    Reference m_reference {CurrentLayer};
    QList<int> m_selectedColorLabels;

    KisPaintDeviceSP m_referencePaintDevice {nullptr};
    KisMergeLabeledLayersCommand::ReferenceNodeInfoListSP m_referenceNodeList {nullptr};
    int m_previousTime {0};
    KisNodeSP m_previousNode {nullptr};
    QSharedPointer<QRect> m_dirtyRect {nullptr};
    KisStrokeId m_fillStrokeId {nullptr};

    KisOptionCollectionWidget *m_optionWidget {nullptr};

    KoGroupButton *m_buttonEnclosingMethodRectangle{nullptr};
    KoGroupButton *m_buttonEnclosingMethodEllipse{nullptr};
    KoGroupButton *m_buttonEnclosingMethodPath{nullptr};
    KoGroupButton *m_buttonEnclosingMethodLasso{nullptr};
    KoGroupButton *m_buttonEnclosingMethodBrush{nullptr};

    QComboBox *m_comboBoxRegionSelectionMethod {nullptr};
    KisColorButton *m_buttonRegionSelectionColor {nullptr};
    QCheckBox *m_checkBoxRegionSelectionInvert {nullptr};
    QCheckBox *m_checkBoxRegionSelectionIncludeContourRegions {nullptr};

    KoGroupButton *m_buttonFillWithFG{nullptr};
    KoGroupButton *m_buttonFillWithBG{nullptr};
    KoGroupButton *m_buttonFillWithPattern{nullptr};
    KisDoubleSliderSpinBox *m_sliderPatternScale {nullptr};
    KisAngleSelector *m_angleSelectorPatternRotation {nullptr};
    QCheckBox *m_checkBoxCustomBlendingOptions {nullptr};
    KisSliderSpinBox *m_sliderCustomOpacity {nullptr};
    KisCompositeOpComboBox *m_comboBoxCustomCompositeOp {nullptr};

    KisSliderSpinBox *m_sliderFillThreshold {nullptr};
    KisSliderSpinBox *m_sliderFillOpacitySpread {nullptr};
    KisSliderSpinBox *m_sliderCloseGap {nullptr};
    QCheckBox *m_checkBoxSelectionAsBoundary {nullptr};

    QCheckBox *m_checkBoxAntiAlias {nullptr};
    KisSliderSpinBox *m_sliderExpand {nullptr};
    QToolButton *m_buttonStopGrowingAtDarkestPixel {nullptr};
    KisSliderSpinBox *m_sliderFeather {nullptr};

    KoGroupButton *m_buttonReferenceCurrent{nullptr};
    KoGroupButton *m_buttonReferenceAll{nullptr};
    KoGroupButton *m_buttonReferenceLabeled{nullptr};
    KisColorLabelSelectorWidget *m_widgetLabels {nullptr};
    QCheckBox *m_checkBoxUseActiveLayer {nullptr};

    KConfigGroup m_configGroup;
    
    bool m_alternateActionStarted {false};

    bool wantsAutoScroll() const override { return false; }

    EnclosingMethod loadEnclosingMethodFromConfig() const;
    void saveEnclosingMethodToConfig(EnclosingMethod enclosingMethod);
    QString enclosingMethodToConfigString(EnclosingMethod enclosingMethod) const;
    EnclosingMethod configStringToEnclosingMethod(const QString &configString) const;
    static constexpr EnclosingMethod defaultEnclosingMethod() { return Lasso; }

    QString regionSelectionMethodToUserString(RegionSelectionMethod regionSelectionMethod) const;
    RegionSelectionMethod loadRegionSelectionMethodFromConfig() const;
    void saveRegionSelectionMethodToConfig(RegionSelectionMethod regionSelectionMethod);
    QString regionSelectionMethodToConfigString(RegionSelectionMethod regionSelectionMethod) const;
    RegionSelectionMethod configStringToRegionSelectionMethod(const QString &configString) const;
    static constexpr RegionSelectionMethod defaultRegionSelectionMethod() { return RegionSelectionMethod::SelectAllRegions; }

    KoColor loadRegionSelectionColorFromConfig();

    QString referenceToUserString(Reference reference) const;
    Reference loadReferenceFromConfig() const;
    void saveReferenceToConfig(Reference reference);
    QString referenceToConfigString(Reference reference) const;
    Reference configStringToReference(const QString &configString) const;
    static constexpr Reference defaultReference() { return CurrentLayer; }

    void setupEnclosingSubtool();
    bool subtoolHasUserInteractionRunning() const;

    void loadConfiguration();

private Q_SLOTS:
    void
    slot_optionButtonStripEnclosingMethod_buttonToggled(KoGroupButton *button,
                                                        bool checked);
    void slot_comboBoxRegionSelectionMethod_currentIndexChanged(int);
    void slot_buttonRegionSelectionColor_changed(const KoColor &color);
    void slot_checkBoxRegionSelectionInvert_toggled(bool checked);
    void slot_checkBoxRegionSelectionIncludeContourRegions_toggled(bool checked);
    void slot_optionButtonStripFillWith_buttonToggled(KoGroupButton *button,
                                                      bool checked);
    void slot_sliderPatternScale_valueChanged(double value);
    void slot_angleSelectorPatternRotation_angleChanged(double value);
    void slot_checkBoxUseCustomBlendingOptions_toggled(bool checked);
    void slot_sliderCustomOpacity_valueChanged(int value);
    void slot_comboBoxCustomCompositeOp_currentIndexChanged(int index);
    void slot_sliderFillThreshold_valueChanged(int value);
    void slot_sliderFillOpacitySpread_valueChanged(int value);
    void slot_sliderCloseGap_valueChanged(int value);
    void slot_checkBoxSelectionAsBoundary_toggled(bool checked);
    void slot_checkBoxAntiAlias_toggled(bool checked);
    void slot_sliderExpand_valueChanged(int value);
    void slot_buttonStopGrowingAtDarkestPixel_toggled(bool enabled);
    void slot_sliderFeather_valueChanged(int value);
    void slot_optionButtonStripReference_buttonToggled(KoGroupButton *button,
                                                       bool checked);
    void slot_widgetLabels_selectionChanged();
    void slot_buttonReset_clicked();

    void slot_currentNodeChanged(const KisNodeSP node);
    void slot_colorSpaceChanged(const KoColorSpace *colorSpace);

    void slot_delegateTool_enclosingMaskProduced(KisPixelSelectionSP enclosingMask);
    void slot_checkBoxUseActiveLayer_toggled(bool checked);

    void resetCursorStyle() override;
};

#endif
