/*
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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


#ifndef KIS_LIGHT_STAGE_h
#define KIS_LIGHT_STAGE_h

#include <QWidget>
#include "kis_light_source.h"
#include <krita_export.h>

class KRITAUI_EXPORT KisLightStage : public QWidget
{
Q_OBJECT
public:
    explicit KisLightStage(QWidget *parent = 0);
    virtual ~KisLightStage();

    virtual void paintEvent(QPaintEvent* event = 0);

    QPoint dragStartPosition;
    QList<KisLightSource> lights;
    QRadialGradient semiSphericalGradient();
    QImage drawPhongBumpmap();
    QRgb illuminatePixel(quint32 posup, quint32 posdown, quint32 posleft, quint32 posright);
    QVector<qreal> heightmap;
    QList<KisLightSource *> lightSources;
    QVector<qreal> semiSphericalHeightmap();

    qreal relativeZIlum;

    bool diffuseLightIsEnabled;
    bool specularLightIsEnabled;

signals:

public slots:
    //void removeLight();
    //void addLight();

};

#endif // KIS_LIGHT_STAGE_h
