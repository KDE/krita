/*
 *  SPDX-FileCopyrightText: 2003-2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_BRUSH_H_
#define KIS_TOOL_BRUSH_H_

#include "kis_tool_freehand.h"

#include <KisSignalMapper.h>

#include "KisToolPaintFactoryBase.h"

#include <flake/kis_node_shape.h>
#include <kis_icon.h>
#include <QKeySequence>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <KoIcon.h>

class QCheckBox;
class QComboBox;
class QLabel;

class KActionCollection;

class KoCanvasBase;
class KisSliderSpinBox;
class KisDoubleSliderSpinBox;

class KisToolBrush : public KisToolFreehand
{
    Q_OBJECT
    Q_PROPERTY(int smoothnessQuality READ smoothnessQuality WRITE slotSetSmoothnessDistance NOTIFY smoothnessQualityChanged)
    Q_PROPERTY(qreal smoothnessFactor READ smoothnessFactor WRITE slotSetTailAgressiveness NOTIFY smoothnessFactorChanged)
    Q_PROPERTY(bool smoothPressure READ smoothPressure WRITE setSmoothPressure NOTIFY smoothPressureChanged)
    Q_PROPERTY(int smoothingType READ smoothingType WRITE slotSetSmoothingType NOTIFY smoothingTypeChanged)
    Q_PROPERTY(bool useScalableDistance READ useScalableDistance WRITE setUseScalableDistance NOTIFY useScalableDistanceChanged)

    Q_PROPERTY(bool useDelayDistance READ useDelayDistance WRITE setUseDelayDistance NOTIFY useDelayDistanceChanged)
    Q_PROPERTY(qreal delayDistance READ delayDistance WRITE setDelayDistance NOTIFY delayDistanceChanged)

    Q_PROPERTY(bool finishStabilizedCurve READ finishStabilizedCurve WRITE setFinishStabilizedCurve NOTIFY finishStabilizedCurveChanged)
    Q_PROPERTY(bool stabilizeSensors READ stabilizeSensors WRITE setStabilizeSensors NOTIFY stabilizeSensorsChanged)


public:
    KisToolBrush(KoCanvasBase * canvas);
    ~KisToolBrush() override;

    QWidget * createOptionWidget() override;

    int smoothnessQuality() const;
    qreal smoothnessFactor() const;
    bool smoothPressure() const;
    int smoothingType() const;
    bool useScalableDistance() const;

    bool useDelayDistance() const;
    qreal delayDistance() const;

    bool finishStabilizedCurve() const;
    bool stabilizeSensors() const;

protected:
    KConfigGroup m_configGroup; // only used in the multihand tool for now

protected Q_SLOTS:
    void resetCursorStyle() override;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void slotSetSmoothnessDistance(qreal distance);
    void slotSetMagnetism(int magnetism);
    void slotSetSmoothingType(int index);
    void slotSetTailAgressiveness(qreal argh_rhhrr);
    void setSmoothPressure(bool value);
    void setUseScalableDistance(bool value);

    void setUseDelayDistance(bool value);
    void setDelayDistance(qreal value);

    void setStabilizeSensors(bool value);

    void setFinishStabilizedCurve(bool value);

    void updateSettingsViews() override;

Q_SIGNALS:
    void smoothnessQualityChanged();
    void smoothnessFactorChanged();
    void smoothPressureChanged();
    void smoothingTypeChanged();
    void useScalableDistanceChanged();

    void useDelayDistanceChanged();
    void delayDistanceChanged();
    void finishStabilizedCurveChanged();
    void stabilizeSensorsChanged();

private:
    void addSmoothingAction(int enumId, const QString &id);
    void updateSmoothnessDistanceLabel();
private:
    QComboBox *m_cmbSmoothingType {0};

    QCheckBox *m_chkAssistant {0};
    KisSliderSpinBox *m_sliderMagnetism {0};
    QCheckBox *m_chkOnlyOneAssistant {0};
    KisDoubleSliderSpinBox *m_sliderSmoothnessDistance {0};
    QLabel *m_lblSmoothnessDistance {0};
    KisDoubleSliderSpinBox *m_sliderTailAggressiveness {0};
    QCheckBox *m_chkSmoothPressure {0};
    QCheckBox *m_chkUseScalableDistance {0};

    QCheckBox *m_chkStabilizeSensors {0};
    QCheckBox *m_chkDelayDistance {0};
    KisDoubleSliderSpinBox *m_sliderDelayDistance {0};

    QCheckBox *m_chkFinishStabilizedCurve {0};
    KisSignalMapper m_signalMapper;
};


class KisToolBrushFactory : public KisToolPaintFactoryBase
{

public:
    KisToolBrushFactory()
            : KisToolPaintFactoryBase("KritaShape/KisToolBrush") {

        setToolTip(i18n("Freehand Brush Tool"));

        // Temporarily
        setSection(ToolBoxSection::Shape);
        setIconName(koIconNameCStr("krita_tool_freehand"));
        setShortcut(QKeySequence(Qt::Key_B));
        setPriority(0);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    KisToolBrushFactory(const QString &id)
        : KisToolPaintFactoryBase(id)
    {
    }

    ~KisToolBrushFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolBrush(canvas);
    }

    QList<QAction *> createActionsImpl() override;

};


#endif // KIS_TOOL_BRUSH_H_
