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
#include <KoIcon.h>
#include <kshortcut.h>

#include <kconfig.h>
#include <kconfiggroup.h>

class QCheckBox;
class QComboBox;

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
    virtual ~KisToolBrush();

    QWidget * createOptionWidget();

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
    KConfigGroup m_configGroup;

protected slots:
    virtual void resetCursorStyle();

public slots:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    void deactivate();
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

    virtual void updateSettingsViews();

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
    void addSmoothingAction(int enumId, const QString &id, const QString &name, KActionCollection *globalCollection);

private:
    QComboBox *m_cmbSmoothingType;

    QCheckBox *m_chkAssistant;
    KisSliderSpinBox *m_sliderMagnetism;
    KisDoubleSliderSpinBox *m_sliderSmoothnessDistance;
    KisDoubleSliderSpinBox *m_sliderTailAggressiveness;
    QCheckBox *m_chkSmoothPressure;
    QCheckBox *m_chkUseScalableDistance;

    QCheckBox *m_chkStabilizeSensors;
    QCheckBox *m_chkDelayDistance;
    KisDoubleSliderSpinBox *m_sliderDelayDistance;

    QCheckBox *m_chkFinishStabilizedCurve;
    QSignalMapper m_signalMapper;
};


class KisToolBrushFactory : public KoToolFactoryBase
{

public:
    KisToolBrushFactory(const QStringList&)
            : KoToolFactoryBase("KritaShape/KisToolBrush") {

        setToolTip(i18n("Freehand Brush Tool"));

        // Temporarily
        setToolType(TOOL_TYPE_SHAPE);
        setIconName(koIconNameCStr("krita_tool_freehand"));
        setShortcut(KShortcut(Qt::Key_B));
        setPriority(0);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    virtual ~KisToolBrushFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolBrush(canvas);
    }

};


#endif // KIS_TOOL_BRUSH_H_
