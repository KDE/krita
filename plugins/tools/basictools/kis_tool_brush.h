/*
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_BRUSH_H_
#define KIS_TOOL_BRUSH_H_

#include "kis_tool_freehand.h"

#include <QSignalMapper>

#include "KoToolFactoryBase.h"

#include <flake/kis_node_shape.h>
#include <kis_icon.h>
#include <QKeySequence>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <KoIcon.h>
#include <QSlider>
#include "kis_tool_brush_tooloptionswidget.h"

class QCheckBox;
class QComboBox;

class KActionCollection;

class KoCanvasBase;
class KisSliderSpinBox;
class KisDoubleSliderSpinBox;

/**
  The stabilizer will have certain presets that range from none to strong
  This is the data structure that will help creating these presets
  */
typedef struct BrushStabilizerSetting {
    // list options a seting type can have
    int smoothingType = 1; // 0 = none, 1 = basic, 2 = weighted, 3 = stabilizer
    float distance = 0.0;
    float strokeEnding  = 0.0;
    float delayDistance  = 0.0;
    bool hasSmoothPressure = false;
    bool hasScalableDistance = false;
    bool hasDelay = false;
    bool hasFinishLine = false;
    bool hasStabilizerSensors = false;
} BrushStabilizerSetting;

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

    /// each smoothing/stabilizer type has its own set of settings. This will show/hide UI
    /// element based off whatever smoothing type is selected
    void updateStabilizerSettingsVisibility(int smoothingTypeIndex);

protected:
    KConfigGroup m_configGroup; // only used in the multihand tool for now

protected Q_SLOTS:
    void resetCursorStyle() override;

    /// toggles whethe we are using stabilizer presets or manually setting values
    void slotCustomSettingsChecked(bool checked);

public Q_SLOTS:
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void slotSetSmoothnessDistance(qreal distance);
    void slotSetMagnetism(int magnetism);
    void slotSetSmoothingType(int index);
    void slotSetTailAgressiveness(qreal argh_rhhrr);

    /// toggled we enable or disable using assistants on the canvas to paint with
    void slotUseAssistants(bool isUsing);
    void setSmoothPressure(bool value);
    void setUseScalableDistance(bool value);

    /// the stabilizer slider that determines the
    /// stabilizer preset we are using
    void slotStabilizerPresetChanged(int value);

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
    void addSmoothingAction(int enumId, const QString &id, const QString &name, const QIcon &icon, KActionCollection *globalCollection);

    /// internal function for modifying the GUI display. It is paired with using custom settings for the stabilizer
    void hideAllStabilizerUIFields();

private:
    QList<BrushStabilizerSetting> stabilizerSettings;

    QSignalMapper m_signalMapper;
    KisToolBrushToolOptionsWidget *m_toolBrushOptions = 0;
};

class KisToolBrushFactory : public KoToolFactoryBase
{

public:
    KisToolBrushFactory()
            : KoToolFactoryBase("KritaShape/KisToolBrush") {

        setToolTip(i18n("Freehand Brush Tool"));

        // Temporarily
        setSection(TOOL_TYPE_SHAPE);
        setIconName(koIconNameCStr("krita_tool_freehand"));
        setShortcut(QKeySequence(Qt::Key_B));
        setPriority(0);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolBrushFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolBrush(canvas);
    }

};


#endif // KIS_TOOL_BRUSH_H_
