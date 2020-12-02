/*
    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

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

#include <QPair>
#include <QGradient>
#include <QtAlgorithms>

#include "KoColor.h"
#include <resources/KoAbstractGradient.h>
#include <resources/KoResource.h>
#include <kritapigment_export.h>
#include <boost/operators.hpp>

class KoStopGradient;
using KoStopGradientSP = QSharedPointer<KoStopGradient>;

enum KoGradientStopType 
{
    COLORSTOP,
    FOREGROUNDSTOP,
    BACKGROUNDSTOP
};

struct KoGradientStop : public boost::equality_comparable<KoGradientStop>
{
    KoGradientStopType type;
    KoColor color;
    qreal position;

    KoGradientStop(qreal _position = 0.0, KoColor _color = KoColor(), KoGradientStopType _type = COLORSTOP) 
    {
        type = _type;
        color = _color;
        position = _position;
    }

    bool operator == (const KoGradientStop& other) 
    { 
        return this->type == other.type && this->color == other.color && this->position == other.position;
    }



    QString typeString() const 
    {
        switch (type) {
        case COLORSTOP:
            return "color-stop";
        case FOREGROUNDSTOP:
            return "foreground-stop";
        case BACKGROUNDSTOP:
            return "background-stop";
        default:
            return "color-stop";
        }
    }

    static KoGradientStopType typeFromString(QString typestring) {
        if (typestring == "foreground-stop") {
            return FOREGROUNDSTOP;
        } else if (typestring == "background-stop") {
            return BACKGROUNDSTOP;
        } else {
            return COLORSTOP;
        }
    }
};


struct KoGradientStopValueSort
{
    inline bool operator() (const KoGradientStop& a, const KoGradientStop& b) {
        return (a.color.toQColor().valueF() < b.color.toQColor().valueF());
    }
};

/**
 * Resource for colorstop based gradients like SVG gradients
 */
class KRITAPIGMENT_EXPORT KoStopGradient : public KoAbstractGradient, public boost::equality_comparable<KoStopGradient>
{

public:
    
    explicit KoStopGradient(const QString &filename = QString());
    ~KoStopGradient() override;

    bool operator==(const KoStopGradient &rhs) const;

    KoAbstractGradient* clone() const override;

    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;
    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;

    /// reimplemented
    QGradient* toQGradient() const override;

    /// Find stops surrounding position, returns false if position outside gradient
    bool stopsAt(KoGradientStop& leftStop, KoGradientStop& rightStop, qreal t) const;

    /// reimplemented
    void colorAt(KoColor&, qreal t) const override;

    /// Creates KoStopGradient from a QGradient
    static KoStopGradient * fromQGradient(const QGradient * gradient);

    /// Sets the gradient stops
    void setStops(QList<KoGradientStop> stops);
    QList<KoGradientStop> stops() const;    

    /// reimplemented
    bool hasVariableColors() const override;
    /// reimplemented
    void setVariableColors(const KoColor& foreground, const KoColor& background) override;
    /// reimplemented
    void bakeVariableColors(const KoColor& foreground, const KoColor& background) override;

    /// reimplemented
    QString defaultFileExtension() const override;

    /**
     * @brief toXML
     * Convert the gradient to an XML string.
     */
    void toXML(QDomDocument& doc, QDomElement& gradientElt) const;
    /**
     * @brief fromXML
     * convert a gradient from xml.
     * @return a gradient.
     */
    static KoStopGradient fromXML(const QDomElement& elt);

protected:

    QList<KoGradientStop> m_stops;
    bool m_hasVariableStops = false;
    QPointF m_start;
    QPointF m_stop;
    QPointF m_focalPoint;

private:

    void loadSvgGradient(QIODevice *file);
    void parseSvgGradient(const QDomElement& element);
    void parseSvgColor(QColor &color, const QString &s);
};

#endif // KOSTOPGRADIENT_H

