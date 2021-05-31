/*
    SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KOSTOPGRADIENT_H
#define KOSTOPGRADIENT_H

#include <QPair>
#include <QGradient>
#include <QtAlgorithms>

#include "KoColor.h"
#include <resources/KoAbstractGradient.h>
#include <KoResource.h>
#include <kritapigment_export.h>
#include <boost/operators.hpp>

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

struct KoGradientStopHueSort
{
    inline bool operator() (const KoGradientStop& a, const KoGradientStop& b) {
        return (a.color.toQColor().hueF() < b.color.toQColor().hueF());
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
    KoStopGradient(const KoStopGradient &rhs);
    bool operator==(const KoStopGradient &rhs) const;
    KoStopGradient &operator=(const KoStopGradient &rhs) = delete;
    KoResourceSP clone() const override;

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice* dev) const override;

    QPair<QString, QString> resourceType() const override {
        return QPair<QString, QString>(ResourceType::Gradients, ResourceSubType::StopGradients);
    }

    /// reimplemented
    QGradient* toQGradient() const override;

    /// Find stops surrounding position, returns false if position outside gradient
    bool stopsAt(KoGradientStop& leftStop, KoGradientStop& rightStop, qreal t) const;

    /// reimplemented
    void colorAt(KoColor&, qreal t) const override;

    /// Creates KoStopGradient from a QGradient
    static QSharedPointer<KoStopGradient> fromQGradient(const QGradient *gradient);

    /// Sets the gradient stops
    void setStops(QList<KoGradientStop> stops);
    QList<KoGradientStop> stops() const;    

    QList<int> requiredCanvasResources() const override;
    void bakeVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface) override;
    void updateVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface) override;


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

    QString saveSvgGradient() const;

protected:

    QList<KoGradientStop> m_stops;
    bool m_hasVariableStops = false;
    QPointF m_start;
    QPointF m_stop;
    QPointF m_focalPoint;

private:

    void loadSvgGradient(QIODevice *file);
    void parseSvgGradient(const QDomElement& element, QHash<QString, const KoColorProfile*> profiles);
};

typedef QSharedPointer<KoStopGradient> KoStopGradientSP;

#endif // KOSTOPGRADIENT_H

