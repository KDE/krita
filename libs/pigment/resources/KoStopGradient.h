/*
    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef KOSTOPGRADIENT_H
#define KOSTOPGRADIENT_H

#include <QtCore/QPair>
#include <QtGui/QGradient>

#include "KoColor.h"
#include "KoAbstractGradient.h"
#include "KoResource.h"
#include <pigment_export.h>

class QFile;

typedef QPair<qreal, KoColor> KoGradientStop;

/**
 * Resource for colorstop based gradients like Karbon gradients and SVG gradients
 */
class PIGMENTCMS_EXPORT KoStopGradient : public KoAbstractGradient
{

public:
    KoStopGradient(const QString& filename);
    virtual ~KoStopGradient();

    virtual bool load();
    /// Not implemented
    virtual bool save();

    /// reimplemented
    virtual QGradient* toQGradient() const;

    /// reimplemented
    void colorAt(KoColor&, qreal t) const;

    /// Creates KoStopGradient from a QGradient
    static KoStopGradient * fromQGradient(QGradient * gradient);
    
    /// Sets the gradient stops
    void setStops(QList<KoGradientStop> stops);

    /// reimplemented
    QString defaultFileExtension() const;

protected:
    QList<KoGradientStop> m_stops;
    QPointF m_start;
    QPointF m_stop;
    QPointF m_focalPoint;
private:
    mutable KoColor buffer;
private:
    void loadKarbonGradient(QFile* file);
    void parseKarbonGradient(const QDomElement& element);

    void loadSvgGradient(QFile* file);
    void parseSvgGradient(const QDomElement& element);
    void parseSvgColor(QColor &color, const QString &s);
};

#endif // KOSTOPGRADIENT_H

