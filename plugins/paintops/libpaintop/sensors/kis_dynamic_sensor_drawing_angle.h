/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DYNAMIC_SENSOR_DRAWING_ANGLE_H
#define __KIS_DYNAMIC_SENSOR_DRAWING_ANGLE_H

#include "kis_dynamic_sensor.h"

class QCheckBox;
class KisSliderSpinBox;


class KisDynamicSensorDrawingAngle : public QObject, public KisDynamicSensor
{
    Q_OBJECT
public:
    KisDynamicSensorDrawingAngle();
    qreal value(const KisPaintInformation& info) override;
    bool dependsOnCanvasRotation() const override;
    bool isAbsoluteRotation() const override;

    QWidget* createConfigurationWidget(QWidget* parent, QWidget*) override;

    using KisSerializableConfiguration::fromXML;
    using KisSerializableConfiguration::toXML;
    void toXML(QDomDocument&, QDomElement&) const override;
    void fromXML(const QDomElement&) override;

    bool fanCornersEnabled() const;
    int fanCornersStep() const;
    int angleOffset() const;

    void reset() override;

public Q_SLOTS:
    void setFanCornersEnabled(int state);
    void setFanCornersStep(int angle);
    void setAngleOffset(int angle);
    void setLockedAngleMode(int value);

    void updateGUI();

private:
    bool m_fanCornersEnabled;
    int m_fanCornersStep;
    int m_angleOffset; // in degrees
    qreal m_lockedAngle;
    bool m_lockedAngleMode;

    QCheckBox *m_chkLockedMode;
    QCheckBox *m_chkFanCorners;
    KisSliderSpinBox *m_intFanCornersStep;
};

#endif /* __KIS_DYNAMIC_SENSOR_DRAWING_ANGLE_H */
