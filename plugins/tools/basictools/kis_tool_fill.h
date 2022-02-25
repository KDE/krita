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

class QWidget;
class QCheckBox;
class KisSliderSpinBox;
class KisDoubleSliderSpinBox;
class KoCanvasBase;
class KisColorFilterCombo;
class KisDummiesFacadeBase;
class KisAngleSelector;

class KisToolFill : public KisToolPaint
{
    Q_OBJECT

public:
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

    QWidget * createOptionWidget() override;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void slotSetUseFastMode(bool);
    void slotSetThreshold(int);
    void slotSetOpacitySpread(int);
    void slotSetUsePattern(bool);
    void slotSetFillSelection(bool);
    void slotSetUseSelectionAsBoundary(bool);
    void slotSetAntiAlias(bool antialias);
    void slotSetSizemod(int);
    void slotSetFeather(int);
    void slotSetSampleLayers(int index);
    void slotSetSelectedColorLabels();
    void slotSetPatternScale(qreal scale);
    void slotSetPatternRotation(qreal rotate);
    void slotSetContinuousFillMode(ContinuousFillMode continuousFillMode);

protected Q_SLOTS:
    void resetCursorStyle() override;
    void slotUpdateAvailableColorLabels();
    void slotUpdateContinuousFill();

protected:
    bool wantsAutoScroll() const override { return false; }
private:
    void updateGUI();
    QString sampleLayerModeToUserString(QString sampleLayersModeId);
    void setCmbSampleLayersMode(QString sampleLayersModeId);

    void activateConnectionsToImage();
    void deactivateConnectionsToImage();

    void beginFilling(const QPoint &seedPoint);
    void addFillingOperation(const QPoint &seedPoint);
    void addFillingOperation(const QVector<QPoint> &seedPoints);
    void addUpdateOperation();
    void endFilling();

private:
    QString SAMPLE_LAYERS_MODE_CURRENT = {"currentLayer"};
    QString SAMPLE_LAYERS_MODE_ALL = {"allLayers"};
    QString SAMPLE_LAYERS_MODE_COLOR_LABELED = {"colorLabeledLayers"};
    static constexpr int minimumDragDistance{4};
    static constexpr int minimumDragDistanceSquared{minimumDragDistance * minimumDragDistance};

    bool m_antiAlias;
    int m_feather;
    int m_sizemod;
    int m_threshold;
    int m_opacitySpread;
    bool m_usePattern;
    bool m_fillOnlySelection;
    bool m_useSelectionAsBoundary;
    bool m_useFastMode;
    QString m_sampleLayersMode;
    QList<int> m_selectedColors;
    qreal m_patternRotation;
    qreal m_patternScale;
    ContinuousFillMode m_continuousFillMode;
    KisSelectionSP m_continuousFillMask;
    KoColor m_continuousFillReferenceColor;
    KisPaintDeviceSP m_referencePaintDevice;
    KisResourcesSnapshotSP m_resourcesSnapshot;
    QTransform m_transform;

    QCheckBox *m_checkUseFastMode;
    KisSliderSpinBox *m_slThreshold;
    KisSliderSpinBox *m_slOpacitySpread;
    QCheckBox *m_checkAntiAlias;
    KisSliderSpinBox *m_sizemodWidget;
    KisSliderSpinBox *m_featherWidget;
    KisAngleSelector *m_angleSelectorPatternRotate;
    KisDoubleSliderSpinBox *m_sldPatternScale;
    QComboBox *m_cmbContinuousFillMode;
    QCheckBox *m_checkUsePattern;
    QCheckBox *m_checkFillSelection;
    QCheckBox *m_checkUseSelectionAsBoundary;
    QComboBox *m_cmbSampleLayersMode;
    KisColorFilterCombo *m_cmbSelectedLabels;
    KisSignalCompressor m_colorLabelCompressor;
    KisDummiesFacadeBase* m_dummiesFacade;
    KisSignalAutoConnectionsStore m_imageConnections;

    bool m_widgetsInitialized{false};
    bool m_isFilling{false};
    bool m_isDragging{false};
    QPoint m_fillStartWidgetPosition;
    KisSignalCompressor m_compressorContinuousFillUpdate;
    QVector<QPoint> m_seedPoints;
    KisStrokeId m_fillStrokeId;

    KConfigGroup m_configGroup;
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

