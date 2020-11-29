/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QList>

#include <KoColorSpaceRegistry.h>

#include "KisGradientConversion.h"

namespace KisGradientConversion
{
    QGradientStops toQGradientStops(KoAbstractGradient *gradient)
    {
        if (!gradient) {
            return QGradientStops();
        }
        if (dynamic_cast<KoStopGradient*>(gradient)) {
            return toQGradientStops(dynamic_cast<KoStopGradient*>(gradient));
        } else if (dynamic_cast<KoSegmentGradient*>(gradient)) {
            return toQGradientStops(dynamic_cast<KoSegmentGradient*>(gradient));
        }
        return QGradientStops();
    }

    QGradientStop toQGradientStop(const KoColor &color, KoGradientStopType type, qreal position)
    {
        Q_UNUSED(type);

        QGradientStop stop;
        stop.first = position;
        color.toQColor(&stop.second);

        return stop;
    }

    QGradientStops toQGradientStops(KoStopGradient *gradient)
    {
        QGradientStops stops;

        if (!gradient) {
            return stops;
        }

        qreal lastStopPosition = -1.0;
        for (const KoGradientStop &stopGradientStop : gradient->stops()) {
            if (qFuzzyCompare(stopGradientStop.position, lastStopPosition)) {
                stops << toQGradientStop(stopGradientStop.color, stopGradientStop.type, stopGradientStop.position + 0.000001);
                lastStopPosition = stopGradientStop.position + 0.000001;
            } else {
                stops << toQGradientStop(stopGradientStop.color, stopGradientStop.type, stopGradientStop.position);
                lastStopPosition = stopGradientStop.position;
            }
        }
        return stops;
    }

    QGradientStop toQGradientStop(const KoColor &color, KoGradientSegmentEndpointType type, qreal offset)
    {
        QGradientStop stop;
        stop.first = offset;
        color.toQColor(&stop.second);

        if (type == FOREGROUND_TRANSPARENT_ENDPOINT || type == BACKGROUND_TRANSPARENT_ENDPOINT) {
            stop.second.setAlpha(0);
        }

        return stop;
    }

    QGradientStops toQGradientStops(KoSegmentGradient *gradient)
    {
        QGradientStops stops;

        if (!gradient) {
            return stops;
        }

        QGradientStop lastStop;
        lastStop.first = -1.0;

        for (KoGradientSegment *segment : gradient->segments()) {
            QGradientStop stop;
            
            stop = toQGradientStop(
                segment->startColor(), segment->startType(), segment->startOffset()
            );
            if (qFuzzyCompare(stop.first, lastStop.first)) {
                if (stop.second != lastStop.second) {
                    stop.first = stop.first + 0.000001;
                    stops << stop;
                    lastStop = stop;
                }
            } else {
                stops << stop;
                lastStop = stop;
            }
            
            stop = toQGradientStop(
                segment->endColor(), segment->endType(), segment->endOffset()
            );
            if (qFuzzyCompare(stop.first, lastStop.first)) {
                if (stop.second != lastStop.second) {
                    stop.first = stop.first + 0.000001;
                    stops << stop;
                    lastStop = stop;
                }
            } else {
                stops << stop;
                lastStop = stop;
            }
        }

        return stops;
    }
    
    QGradientStops toQGradientStops(KoAbstractGradient *gradient,
                                    const KoColor &foregroundColor,
                                    const KoColor &backgroundcolor)
    {
        if (!gradient) {
            return QGradientStops();
        }
        if (dynamic_cast<KoStopGradient*>(gradient)) {
            return toQGradientStops(dynamic_cast<KoStopGradient*>(gradient), foregroundColor, backgroundcolor);
        } else if (dynamic_cast<KoSegmentGradient*>(gradient)) {
            return toQGradientStops(dynamic_cast<KoSegmentGradient*>(gradient), foregroundColor, backgroundcolor);
        }
        return QGradientStops();
    }

    QGradientStop toQGradientStop(const KoColor &color, KoGradientStopType type, qreal position,
                                  const KoColor &foregroundColor,
                                  const KoColor &backgroundcolor)
    {
        QGradientStop stop;
        stop.first = position;

        if (type == FOREGROUNDSTOP) {
            foregroundColor.toQColor(&stop.second);
        } else if (type == BACKGROUNDSTOP) {
            backgroundcolor.toQColor(&stop.second);
        } else {
            color.toQColor(&stop.second);
        }

        return stop;
    }

    QGradientStops toQGradientStops(KoStopGradient *gradient,
                                    const KoColor &foregroundColor,
                                    const KoColor &backgroundcolor)
    {
        QGradientStops stops;

        if (!gradient) {
            return stops;
        }

        qreal lastStopPosition = -1.0;
        for (const KoGradientStop &stopGradientStop : gradient->stops()) {
            if (qFuzzyCompare(stopGradientStop.position, lastStopPosition)) {
                stops << toQGradientStop(
                    stopGradientStop.color, stopGradientStop.type, stopGradientStop.position + 0.000001,
                foregroundColor, backgroundcolor
                );
                lastStopPosition = stopGradientStop.position + 0.000001;
            } else {
                stops << toQGradientStop(
                    stopGradientStop.color, stopGradientStop.type, stopGradientStop.position,
                    foregroundColor, backgroundcolor
                );
                lastStopPosition = stopGradientStop.position;
            }
        }
        return stops;
    }

    QGradientStop toQGradientStop(const KoColor &color, KoGradientSegmentEndpointType type, qreal offset,
                                  const KoColor &foregroundColor,
                                  const KoColor &backgroundcolor)
    {
        QGradientStop stop;
        stop.first = offset;

        if (type == FOREGROUND_ENDPOINT) {
            foregroundColor.toQColor(&stop.second);
        } else if (type == FOREGROUND_TRANSPARENT_ENDPOINT) {
            foregroundColor.toQColor(&stop.second);
            stop.second.setAlpha(0);
                return stop;
        } else if (type == BACKGROUND_ENDPOINT) {
            backgroundcolor.toQColor(&stop.second);
        } else if (type == BACKGROUND_TRANSPARENT_ENDPOINT) {
            backgroundcolor.toQColor(&stop.second);
            stop.second.setAlpha(0);
        } else {
            color.toQColor(&stop.second);
        }
        
        return stop;
    }

    QGradientStops toQGradientStops(KoSegmentGradient *gradient,
                                    const KoColor &foregroundColor,
                                    const KoColor &backgroundcolor)
    {
        QGradientStops stops;

        if (!gradient) {
            return stops;
        }

        QGradientStop lastStop;
        lastStop.first = -1.0;

        for (KoGradientSegment *segment : gradient->segments()) {
            QGradientStop stop;
            
            stop = toQGradientStop(
                segment->startColor(), segment->startType(), segment->startOffset(),
                foregroundColor, backgroundcolor
            );
            if (qFuzzyCompare(stop.first, lastStop.first)) {
                if (stop.second != lastStop.second) {
                    stop.first = stop.first + 0.000001;
                    stops << stop;
                    lastStop = stop;
                }
            } else {
                stops << stop;
                lastStop = stop;
            }
            
            stop = toQGradientStop(
                segment->endColor(), segment->endType(), segment->endOffset(),
                foregroundColor, backgroundcolor
            );
            if (qFuzzyCompare(stop.first, lastStop.first)) {
                if (stop.second != lastStop.second) {
                    stop.first = stop.first + 0.000001;
                    stops << stop;
                    lastStop = stop;
                }
            } else {
                stops << stop;
                lastStop = stop;
            }
        }

        return stops;
    }
    
    QGradient* toQGradient(KoAbstractGradient *gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        if (dynamic_cast<KoStopGradient*>(gradient)) {
            return toQGradient(dynamic_cast<KoStopGradient*>(gradient));
        } else if (dynamic_cast<KoSegmentGradient*>(gradient)) {
            return toQGradient(dynamic_cast<KoSegmentGradient*>(gradient));
        }
        return nullptr;
    }

    QGradient* toQGradient(KoStopGradient *gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        QGradient *qGradient = new QLinearGradient;
        qGradient->setStops(toQGradientStops(gradient));
        return qGradient;
    }

    QGradient* toQGradient(KoSegmentGradient *gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        QGradient *qGradient = new QLinearGradient;
        qGradient->setStops(toQGradientStops(gradient));
        return qGradient;
    }

    QGradient* toQGradient(KoAbstractGradient *gradient, const KoColor &foregroundColor, const KoColor &backgroundcolor)
    {
        if (!gradient) {
            return nullptr;
        }
        if (dynamic_cast<KoStopGradient*>(gradient)) {
            return toQGradient(dynamic_cast<KoStopGradient*>(gradient), foregroundColor, backgroundcolor);
        } else if (dynamic_cast<KoSegmentGradient*>(gradient)) {
            return toQGradient(dynamic_cast<KoSegmentGradient*>(gradient), foregroundColor, backgroundcolor);
        }
        return nullptr;
    }

    QGradient* toQGradient(KoStopGradient *gradient, const KoColor &foregroundColor, const KoColor &backgroundcolor)
    {
        if (!gradient) {
            return nullptr;
        }
        QGradient *qGradient = new QLinearGradient;
        qGradient->setStops(toQGradientStops(gradient, foregroundColor, backgroundcolor));
        return qGradient;
    }

    QGradient* toQGradient(KoSegmentGradient *gradient, const KoColor &foregroundColor, const KoColor &backgroundcolor)
    {
        if (!gradient) {
            return nullptr;
        }
        QGradient *qGradient = new QLinearGradient;
        qGradient->setStops(toQGradientStops(gradient, foregroundColor, backgroundcolor));
        return qGradient;
    }

    KoAbstractGradient* toAbstractGradient(const QGradientStops &gradient)
    {
        return dynamic_cast<KoAbstractGradient*>(toStopGradient(gradient));
    }

    KoAbstractGradient* toAbstractGradient(const QGradient *gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        return dynamic_cast<KoAbstractGradient*>(toStopGradient(gradient));
    }

    KoAbstractGradient* toAbstractGradient(KoStopGradient *gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        return gradient->clone();
    }

    KoAbstractGradient* toAbstractGradient(KoSegmentGradient *gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        return gradient->clone();
    }

    KoStopGradient* toStopGradient(const QGradientStops &gradient)
    {
        KoStopGradient *stopGradient = new KoStopGradient;
        QList<KoGradientStop> stops;

        for (const QGradientStop &qGradientStop : gradient) {
            KoGradientStop stop;
            stop.type = COLORSTOP;
            stop.position = qGradientStop.first;
            stop.color = KoColor(qGradientStop.second, stopGradient->colorSpace());
            stops << stop;
        }

        stopGradient->setStops(stops);
        stopGradient->setType(QGradient::LinearGradient);
        stopGradient->setValid(true);

        return stopGradient;
    }

    KoStopGradient* toStopGradient(const QGradient *gradient)
    {
        if (!gradient || gradient->type() == QGradient::NoGradient) {
            return nullptr;
        }
        KoStopGradient *stopGradient = toStopGradient(gradient->stops());
        stopGradient->setType(gradient->type());
        stopGradient->setSpread(gradient->spread());
        return stopGradient;
    }

    KoStopGradient* toStopGradient(KoAbstractGradient *gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        if (dynamic_cast<KoStopGradient*>(gradient)) {
            return dynamic_cast<KoStopGradient*>(gradient->clone());
        } else if (dynamic_cast<KoSegmentGradient*>(gradient)) {
            return toStopGradient(dynamic_cast<KoSegmentGradient*>(gradient));
        }
        return nullptr;
    }

    KoStopGradient* toStopGradient(KoAbstractGradient *gradient,
                                    const KoColor &foregroundColor,
                                    const KoColor &backgroundcolor)
    {
        if (!gradient) {
            return nullptr;
        }
        if (dynamic_cast<KoStopGradient*>(gradient)) {
            return dynamic_cast<KoStopGradient*>(gradient->clone());
        } else if (dynamic_cast<KoSegmentGradient*>(gradient)) {
            return toStopGradient(dynamic_cast<KoSegmentGradient*>(gradient), foregroundColor, backgroundcolor);
        }
        return nullptr;
    }

    KoGradientStop toKoGradientStop(const KoColor &color, KoGradientSegmentEndpointType type, qreal offset)
    {
        KoGradientStop stop;
    
        stop.color = color;
        stop.position = offset;

        if (type == FOREGROUND_ENDPOINT) {
            stop.type = FOREGROUNDSTOP;
        } else if (type == BACKGROUND_ENDPOINT) {
            stop.type = BACKGROUNDSTOP;
        } else if (type == FOREGROUND_TRANSPARENT_ENDPOINT || type == BACKGROUND_TRANSPARENT_ENDPOINT) {
            stop.type = COLORSTOP;
            stop.color.setOpacity(static_cast<quint8>(0));
        } else {
            stop.type = COLORSTOP;
        }

        return stop;
    }

    KoStopGradient* toStopGradient(KoSegmentGradient *gradient)
    {
        if (!gradient) {
            return nullptr;
        }

        KoStopGradient *stopGradient = new KoStopGradient;
        QList<KoGradientStop> stops;

        KoGradientStop lastStop;
        lastStop.position = -1.0;

        for (KoGradientSegment *segment : gradient->segments()) {
            KoGradientStop stop;
            
            stop = toKoGradientStop(
                segment->startColor(), segment->startType(), segment->startOffset()
            );
            stop.color.convertTo(stopGradient->colorSpace());
            if (!qFuzzyCompare(stop.position, lastStop.position) || stop.type != lastStop.type || stop.color != lastStop.color) {
                stops << stop;
                lastStop.type = stop.type;
                lastStop.color = stop.color;
                lastStop.position = stop.position;
            }
            
            stop = toKoGradientStop(
                segment->endColor(), segment->endType(), segment->endOffset()
            );
            stop.color.convertTo(stopGradient->colorSpace());
            if (!qFuzzyCompare(stop.position, lastStop.position) || stop.type != lastStop.type || stop.color != lastStop.color) {
                stops << stop;
                lastStop.type = stop.type;
                lastStop.color = stop.color;
                lastStop.position = stop.position;
            }
        }

        stopGradient->setType(gradient->type());
        stopGradient->setSpread(gradient->spread());
        stopGradient->setStops(stops);

        stopGradient->setName(gradient->name());
        stopGradient->setFilename(gradient->filename());
        stopGradient->setValid(true);

        return stopGradient;
    }

    KoGradientStop toKoGradientStop(const KoColor &color, KoGradientSegmentEndpointType type, qreal offset,
                                    const KoColor &foregroundColor,
                                    const KoColor &backgroundcolor)
    {
        KoGradientStop stop;
    
        stop.position = offset;

        if (type == FOREGROUND_ENDPOINT) {
            stop.type = FOREGROUNDSTOP;
            stop.color = color;
        } else if (type == FOREGROUND_TRANSPARENT_ENDPOINT) {
            stop.type = COLORSTOP;
            stop.color = foregroundColor;
            stop.color.setOpacity(static_cast<quint8>(0));
        } else if (type == BACKGROUND_ENDPOINT) {
            stop.type = BACKGROUNDSTOP;
            stop.color = color;
        } else if (type == BACKGROUND_TRANSPARENT_ENDPOINT) {
            stop.type = COLORSTOP;
            stop.color = backgroundcolor;
            stop.color.setOpacity(static_cast<quint8>(0));
        } else {
            stop.type = COLORSTOP;
            stop.color = color;
        }

        return stop;
    }

    KoStopGradient* toStopGradient(KoSegmentGradient *gradient,
                                   const KoColor &foregroundColor,
                                   const KoColor &backgroundcolor)
    {
        if (!gradient) {
            return nullptr;
        }

        KoStopGradient *stopGradient = new KoStopGradient;
        QList<KoGradientStop> stops;

        KoGradientStop lastStop;
        lastStop.position = -1.0;

        for (KoGradientSegment *segment : gradient->segments()) {
            KoGradientStop stop;
            
            stop = toKoGradientStop(
                segment->startColor(), segment->startType(), segment->startOffset(),
                foregroundColor, backgroundcolor
            );
            stop.color.convertTo(stopGradient->colorSpace());
            if (!qFuzzyCompare(stop.position, lastStop.position) || stop.type != lastStop.type || stop.color != lastStop.color) {
                stops << stop;
                lastStop.type = stop.type;
                lastStop.color = stop.color;
                lastStop.position = stop.position;
            }
            
            stop = toKoGradientStop(
                segment->endColor(), segment->endType(), segment->endOffset(),
                foregroundColor, backgroundcolor
            );
            stop.color.convertTo(stopGradient->colorSpace());
            if (!qFuzzyCompare(stop.position, lastStop.position) || stop.type != lastStop.type || stop.color != lastStop.color) {
                stops << stop;
                lastStop.type = stop.type;
                lastStop.color = stop.color;
                lastStop.position = stop.position;
            }
        }

        stopGradient->setType(gradient->type());
        stopGradient->setSpread(gradient->spread());
        stopGradient->setStops(stops);

        stopGradient->setName(gradient->name());
        stopGradient->setFilename(gradient->filename());
        stopGradient->setValid(true);

        return stopGradient;
    }

    KoSegmentGradient* toSegmentGradient(const QGradientStops &gradient)
    {
        KoSegmentGradient *segmentGradient = new KoSegmentGradient;
        const QGradientStops &stops = gradient;

        for (int i = 0; i < stops.size() - 1; ++i) {
            if (qFuzzyCompare(stops[i].first, stops[i + 1].first)) {
                continue;
            }
            segmentGradient->createSegment(
                INTERP_LINEAR, COLOR_INTERP_RGB,
                stops[i].first, stops[i + 1].first, (stops[i].first + stops[i + 1].first) / 2,
                stops[i].second, stops[i + 1].second
            );
        }

        segmentGradient->setValid(true);
        
        return segmentGradient;
    }

    KoSegmentGradient* toSegmentGradient(const QGradient *gradient)
    {
        if (!gradient || gradient->type() == QGradient::NoGradient) {
            return nullptr;
        }
        KoSegmentGradient *segmentGradient = toSegmentGradient(gradient->stops());
        segmentGradient->setType(gradient->type());
        segmentGradient->setSpread(gradient->spread());
        return segmentGradient;
    }

    KoSegmentGradient* toSegmentGradient(KoAbstractGradient* gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        if (dynamic_cast<KoSegmentGradient*>(gradient)) {
            return dynamic_cast<KoSegmentGradient*>(gradient->clone());
        } else if (dynamic_cast<KoStopGradient*>(gradient)) {
            return toSegmentGradient(dynamic_cast<KoStopGradient*>(gradient));
        }
        return nullptr;
    }

    KoSegmentGradient* toSegmentGradient(KoStopGradient *gradient)
    {
        if (!gradient) {
            return nullptr;
        }

        KoSegmentGradient *segmentGradient = new KoSegmentGradient;
        QList<KoGradientStop> stops = gradient->stops();

        for (int i = 0; i < stops.size() - 1; ++i) {
            if (qFuzzyCompare(stops[i].position, stops[i + 1].position)) {
                continue;
            }

            KoGradientSegmentEndpointType startType, endType;
            const KoGradientStopType startStopType = stops[i].type;
            const KoGradientStopType endStopType = stops[i + 1].type;

            if (startStopType == FOREGROUNDSTOP) {
                startType = FOREGROUND_ENDPOINT;
            } else if (startStopType == BACKGROUNDSTOP) {
                startType = BACKGROUND_ENDPOINT;
            } else {
                startType = COLOR_ENDPOINT;
            }

            if (endStopType == FOREGROUNDSTOP) {
                endType = FOREGROUND_ENDPOINT;
            } else if (endStopType == BACKGROUNDSTOP) {
                endType = BACKGROUND_ENDPOINT;
            } else {
                endType = COLOR_ENDPOINT;
            }
            
            segmentGradient->createSegment(
                INTERP_LINEAR, COLOR_INTERP_RGB,
                stops[i].position, stops[i + 1].position, (stops[i].position + stops[i + 1].position) / 2,
                stops[i].color.toQColor(), stops[i + 1].color.toQColor(),
                startType, endType
            );
        }

        segmentGradient->setType(gradient->type());
        segmentGradient->setSpread(gradient->spread());

        segmentGradient->setName(gradient->name());
        segmentGradient->setFilename(gradient->filename());
        segmentGradient->setValid(true);

        return segmentGradient;
    }
}
