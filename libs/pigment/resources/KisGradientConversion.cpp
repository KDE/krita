/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QList>

#include <KoCanvasResourcesIds.h>
#include <KoColorSpaceRegistry.h>

#include "KisGradientConversion.h"

namespace KisGradientConversion
{
    QGradientStops toQGradientStops(KoAbstractGradientSP gradient,
                                    KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    {
        if (!gradient) {
            return QGradientStops();
        }
        if (gradient.dynamicCast<KoStopGradient>()) {
            return toQGradientStops(gradient.dynamicCast<KoStopGradient>(), canvasResourcesInterface);
        } else if (gradient.dynamicCast<KoSegmentGradient>()) {
            return toQGradientStops(gradient.dynamicCast<KoSegmentGradient>(), canvasResourcesInterface);
        }
        return QGradientStops();
    }

    QGradientStop toQGradientStop(const KoColor &color, KoGradientStopType type, qreal position,
                                  KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    {
        QGradientStop stop;
        stop.first = position;

        if (type == FOREGROUNDSTOP) {
            if (canvasResourcesInterface) {
                canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().toQColor(&stop.second);
                return stop;
            }
        } else if (type == BACKGROUNDSTOP) {
            if (canvasResourcesInterface) {
                canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().toQColor(&stop.second);
                return stop;
            }
        }
        
        color.toQColor(&stop.second);
        return stop;
    }

    QGradientStops toQGradientStops(KoStopGradientSP gradient,
                                    KoCanvasResourcesInterfaceSP canvasResourcesInterface)
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
                    canvasResourcesInterface
                );
                lastStopPosition = stopGradientStop.position + 0.000001;
            } else {
                stops << toQGradientStop(
                    stopGradientStop.color, stopGradientStop.type, stopGradientStop.position,
                    canvasResourcesInterface
                );
                lastStopPosition = stopGradientStop.position;
            }
        }
        return stops;
    }

    QGradientStop toQGradientStop(const KoColor &color, KoGradientSegmentEndpointType type, qreal offset,
                                  KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    {
        QGradientStop stop;
        stop.first = offset;

        if (type == FOREGROUND_ENDPOINT) {
            if (canvasResourcesInterface) {
                canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().toQColor(&stop.second);
                return stop;
            }
        } else if (type == FOREGROUND_TRANSPARENT_ENDPOINT) {
            if (canvasResourcesInterface) {
                canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().toQColor(&stop.second);
                stop.second.setAlpha(0);
                return stop;
            }
        } else if (type == BACKGROUND_ENDPOINT) {
            if (canvasResourcesInterface) {
                canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().toQColor(&stop.second);
                return stop;
            }
        } else if (type == BACKGROUND_TRANSPARENT_ENDPOINT) {
            if (canvasResourcesInterface) {
                canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().toQColor(&stop.second);
                stop.second.setAlpha(0);
                return stop;
            }
        }
        
        color.toQColor(&stop.second);
        return stop;
    }

    QGradientStops toQGradientStops(KoSegmentGradientSP gradient,
                                    KoCanvasResourcesInterfaceSP canvasResourcesInterface)
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
                canvasResourcesInterface
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
                canvasResourcesInterface
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
    
    QGradient* toQGradient(KoAbstractGradientSP gradient, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    {
        if (!gradient) {
            return nullptr;
        }
        if (gradient.dynamicCast<KoStopGradient>()) {
            return toQGradient(gradient.dynamicCast<KoStopGradient>(), canvasResourcesInterface);
        } else if (gradient.dynamicCast<KoSegmentGradient>()) {
            return toQGradient(gradient.dynamicCast<KoSegmentGradient>(), canvasResourcesInterface);
        }
        return nullptr;
    }

    QGradient* toQGradient(KoStopGradientSP gradient, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    {
        if (!gradient) {
            return nullptr;
        }
        QGradient *qGradient = new QLinearGradient;
        qGradient->setStops(toQGradientStops(gradient, canvasResourcesInterface));
        return qGradient;
    }

    QGradient* toQGradient(KoSegmentGradientSP gradient, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    {
        if (!gradient) {
            return nullptr;
        }
        QGradient *qGradient = new QLinearGradient;
        qGradient->setStops(toQGradientStops(gradient, canvasResourcesInterface));
        return qGradient;
    }

    KoAbstractGradientSP toAbstractGradient(const QGradientStops &gradient)
    {
        return toStopGradient(gradient).dynamicCast<KoAbstractGradient>();
    }

    KoAbstractGradientSP toAbstractGradient(const QGradient *gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        return toStopGradient(gradient).dynamicCast<KoAbstractGradient>();
    }

    KoAbstractGradientSP toAbstractGradient(KoStopGradientSP gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        return gradient->clone().dynamicCast<KoAbstractGradient>();
    }

    KoAbstractGradientSP toAbstractGradient(KoSegmentGradientSP gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        return gradient->clone().dynamicCast<KoAbstractGradient>();
    }

    KoStopGradientSP toStopGradient(const QGradientStops &gradient)
    {
        KoStopGradientSP stopGradient(new KoStopGradient);
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

    KoStopGradientSP toStopGradient(const QGradient *gradient)
    {
        if (!gradient || gradient->type() == QGradient::NoGradient) {
            return nullptr;
        }
        KoStopGradientSP stopGradient = toStopGradient(gradient->stops());
        stopGradient->setType(gradient->type());
        stopGradient->setSpread(gradient->spread());
        return stopGradient;
    }

    KoStopGradientSP toStopGradient(KoAbstractGradientSP gradient,
                                    KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    {
        if (!gradient) {
            return nullptr;
        }
        if (gradient.dynamicCast<KoStopGradient>()) {
            return gradient->clone().dynamicCast<KoStopGradient>();
        } else if (gradient.dynamicCast<KoSegmentGradient>()) {
            return toStopGradient(gradient.dynamicCast<KoSegmentGradient>(), canvasResourcesInterface);
        }
        return nullptr;
    }

    KoGradientStop toKoGradientStop(const KoColor &color, KoGradientSegmentEndpointType type, qreal offset,
                                    KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    {
        KoGradientStop stop;
    
        stop.position = offset;

        if (type == FOREGROUND_ENDPOINT) {
            stop.type = FOREGROUNDSTOP;
            stop.color = color;
        } else if (type == FOREGROUND_TRANSPARENT_ENDPOINT) {
            stop.type = COLORSTOP;
            if (canvasResourcesInterface) {
                stop.color = canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
            } else {
                stop.color = color;
            }
            stop.color.setOpacity(static_cast<quint8>(0));
        } else if (type == BACKGROUND_ENDPOINT) {
            stop.type = BACKGROUNDSTOP;
            stop.color = color;
        } else if (type == BACKGROUND_TRANSPARENT_ENDPOINT) {
            stop.type = COLORSTOP;
            if (canvasResourcesInterface) {
                stop.color = canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
            } else {
                stop.color = color;
            }
            stop.color.setOpacity(static_cast<quint8>(0));
        } else {
            stop.type = COLORSTOP;
            stop.color = color;
        }

        return stop;
    }

    KoStopGradientSP toStopGradient(KoSegmentGradientSP gradient, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    {
        if (!gradient) {
            return nullptr;
        }

        KoStopGradientSP stopGradient(new KoStopGradient);
        QList<KoGradientStop> stops;

        KoGradientStop lastStop;
        lastStop.position = -1.0;

        for (KoGradientSegment *segment : gradient->segments()) {
            KoGradientStop stop;
            
            stop = toKoGradientStop(
                segment->startColor(), segment->startType(), segment->startOffset(),
                canvasResourcesInterface
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
                canvasResourcesInterface
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

    KoSegmentGradientSP toSegmentGradient(const QGradientStops &gradient)
    {
        KoSegmentGradientSP segmentGradient(new KoSegmentGradient);
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

    KoSegmentGradientSP toSegmentGradient(const QGradient *gradient)
    {
        if (!gradient || gradient->type() == QGradient::NoGradient) {
            return nullptr;
        }
        KoSegmentGradientSP segmentGradient = toSegmentGradient(gradient->stops());
        segmentGradient->setType(gradient->type());
        segmentGradient->setSpread(gradient->spread());
        return segmentGradient;
    }

    KoSegmentGradientSP toSegmentGradient(KoAbstractGradientSP gradient)
    {
        if (!gradient) {
            return nullptr;
        }
        if (gradient.dynamicCast<KoSegmentGradient>()) {
            return gradient->clone().dynamicCast<KoSegmentGradient>();
        } else if (gradient.dynamicCast<KoStopGradient>()) {
            return toSegmentGradient(gradient.dynamicCast<KoStopGradient>());
        }
        return nullptr;
    }

    KoSegmentGradientSP toSegmentGradient(KoStopGradientSP gradient)
    {
        if (!gradient) {
            return nullptr;
        }

        KoSegmentGradientSP segmentGradient(new KoSegmentGradient);
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
