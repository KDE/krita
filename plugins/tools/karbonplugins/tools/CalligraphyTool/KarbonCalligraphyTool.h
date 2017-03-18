/* This file is part of the KDE project
 * Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KARBONCALLIGRAPHYTOOL_H
#define KARBONCALLIGRAPHYTOOL_H

#include <KoToolBase.h>
#include <kis_tool_shape.h>
#include <KoPathShape.h>
#include <QPointer>
#include <QTime>
#include <kis_painting_information_builder.h>
#include <kis_paint_information.h>
#include <kis_properties_configuration.h>

class KoPathShape;
class KarbonCalligraphicShape;

class KarbonCalligraphyTool : public KisToolShape
{
    Q_OBJECT
public:
    explicit KarbonCalligraphyTool(KoCanvasBase *canvas);
    ~KarbonCalligraphyTool();

    void paint(QPainter &painter, const KoViewConverter &converter);

    /**
     * @brief configuration holds the interpretation of the paintinfo,
     * this is similar to a vector version of a paintop.
     * @return the configuration that is currently held by the object.
     */
    KisPropertiesConfigurationSP configuration();

    void mousePressEvent(KoPointerEvent *event);
    void mouseMoveEvent(KoPointerEvent *event);
    void mouseReleaseEvent(KoPointerEvent *event);

    QList<QPointer<QWidget> > createOptionWidgets();

    virtual void activate(ToolActivation activation, const QSet<KoShape *> &shapes);
    void deactivate();

Q_SIGNALS:
    void pathSelectedChanged(bool selection);

private Q_SLOTS:
    /**
     * @brief setConfiguration
     * Set the configuration of the paintinfo interpretation(the paintop, basically)
     * This will update the full stroke.
     * @param setting
     */
    //void setConfiguration(KisPropertiesConfigurationSP setting) const;
    void setUsePath(bool usePath);
    void setUseAssistant(bool useAssistant);
    void setNoAdjust(bool none);
    void setCaps(double caps);
    /**
     * @brief setSmoothIntervalTime
     * @param time in milliseconds.
     */
    void setSmoothIntervalTime(double time);
    void setSmoothIntervalDistance(double dist);

    void updateSelectedPath();

private:
    void addPoint(KoPointerEvent *event, bool lastPoint = false);
    // auxiliary function that sets m_angle
    //void setAngle(KoPointerEvent *event);
    // auxiliary functions to calculate the dynamic parameters
    // returns the new point and sets speed to the speed
    QPointF calculateNewPoint(const QPointF &mousePos, QPointF firstPathPosition);
    //qreal calculateWidth(qreal pressure);
    //qreal calculateAngle(const QPointF &oldSpeed, const QPointF &newSpeed);

    void smoothPoints();

    QPointF m_lastPoint;
    KarbonCalligraphicShape *m_shape;

    // used to determine if the device supports tilt
    bool m_deviceSupportsTilt;

    bool m_usePath;         // follow selected path
    bool m_useAssistant;
    qreal m_strokeWidth;
    qreal m_caps;
    qreal m_smoothIntervalTime;
    qreal m_smoothIntervalDistance;

    KoPathShape *m_selectedPath;
    QPainterPath m_selectedPathOutline;
    QPointF m_firstPathPosition;
    qreal m_followPathPosition;
    bool m_endOfPath;
    QPointF m_lastMousePos;

    bool m_isDrawing;
    QTime m_strokeTime;
    KisPaintInformation m_lastInfo;
    KisDistanceInformation m_currentDistance;
    KisPaintingInformationBuilder *m_infoBuilder;
    int m_pointCount;

    QList<KisPaintInformation> m_intervalStore;
    QList<KisPaintInformation> m_intervalStoreOld;

    // dynamic parameters
    QPointF m_speed; // used as a vector

    // last calligraphic shape drawn, if any
    KarbonCalligraphicShape *m_lastShape;
};

#endif // KARBONCALLIGRAPHYTOOL_H
