/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef KIS_EXPERIMENT_PAINTOP_H_
#define KIS_EXPERIMENT_PAINTOP_H_

#include <klocale.h>
#include <kis_paintop.h>
#include <kis_types.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>

#include "kis_experiment_paintop_settings.h"
#include "kis_experimentop_option.h"

class QPointF;
class KisPainter;

class KisExperimentPaintOp : public KisPaintOp
{

public:

    KisExperimentPaintOp(const KisExperimentPaintOpSettings *settings, KisPainter * painter, KisImageSP image);
    virtual ~KisExperimentPaintOp();

    virtual KisDistanceInformation paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, const KisDistanceInformation& savedDist = KisDistanceInformation());
    virtual qreal paintAt(const KisPaintInformation& info);

private:
    void fillPainterPath(const QPainterPath &path);
    
private:
    const KisExperimentPaintOpSettings* m_settings;

    KisPaintDeviceSP m_currentLayerDevice;
    
    QVector<QRect> m_previousDabs;
    bool m_isFirst;
    
    QPainterPath m_path;
    QImage m_polygonMaskImage;
    KisFixedPaintDeviceSP m_polygonDevice;
    
    KisPressureRotationOption m_rotationOption;
    KisPressureSizeOption m_sizeOption;
    KisPressureOpacityOption m_opacityOption;
    ExperimentOption m_experimentOption;

    int m_displacement; // 7
    int m_multiplier; 
    bool m_smoothing;
    
    QPainterPath applyDisplace(const QPainterPath& path, int speed);
    QPointF getAngle(const QPointF &p1,const QPointF &p2, double distance);
    int getCursorSpeed(const QPointF &p1,const QPointF &p2){
        int diffX = qAbs(p1.x() - p2.x());
        int diffY = qAbs(p1.y() - p2.y());
        return diffX + diffY;
    }
    
    void curveTo(QPainterPath &path, QPointF pt);
    void addPosition(const QPointF &pos);

    void clearPreviousDab();
    void clearPreviousDab2();
};

#endif // KIS_EXPERIMENT_PAINTOP_H_
