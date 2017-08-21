/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_DISPLAY_FILTER_H
#define KIS_DISPLAY_FILTER_H

#include <QObject>

#include <qopengl.h>

#include <kritaui_export.h>

struct KisExposureGammaCorrectionInterface;

/**
 * @brief The KisDisplayFilter class is the base class for filters that
 * are applied by the canvas to the projection before displaying.
 */
class KRITAUI_EXPORT KisDisplayFilter : public QObject
{
    Q_OBJECT
public:
    explicit KisDisplayFilter(QObject *parent = 0);

    virtual QString program() const = 0;
    virtual GLuint lutTexture() const = 0;
    virtual void filter(quint8 *pixels, quint32 numPixels) = 0;
    virtual void approximateInverseTransformation(quint8 *pixels, quint32 numPixels) = 0;
    virtual void approximateForwardTransformation(quint8 *pixels, quint32 numPixels) = 0;
    virtual bool useInternalColorManagement() const = 0;
    virtual KisExposureGammaCorrectionInterface *correctionInterface() const = 0;
    virtual bool lockCurrentColorVisualRepresentation() const = 0;
    /**
     * @return true if the shader should be recompiled
     */
    virtual bool updateShader() = 0;
};


#endif
