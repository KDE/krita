/* This file is part of the KDE project
   Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoMarkerData.h"

#include <FlakeDebug.h>
#include <KoStyleStack.h>
#include <KoXmlNS.h>
#include <KoUnit.h>
#include <KoOdfLoadingContext.h>
#include <KoGenStyle.h>
#include "KoShapeLoadingContext.h"
#include "KoMarker.h"
#include "KoMarkerSharedLoadingData.h"

/**
 * This defines the factor the width of the arrow is widened 
 * when the width of the line is changed.
 */
static const qreal ResizeFactor = 1.5;

static const struct {
    const char * m_markerPositionLoad;
    const char * m_markerWidthLoad;
    const char * m_markerCenterLoad;
    const char * m_markerPositionSave;
    const char * m_markerWidthSave;
    const char * m_markerCenterSave;
} markerOdfData[] = {
    { "marker-start", "marker-start-width", "marker-start-center", "draw:marker-start", "draw:marker-start-width", "draw:marker-start-center" },
    { "marker-end"  , "marker-end-width",   "marker-end-center", "draw:marker-end"  , "draw:marker-end-width",   "draw:marker-end-center" }
};

class Q_DECL_HIDDEN KoMarkerData::Private
{
public:
    Private(KoMarker *marker, qreal baseWidth, KoMarkerData::MarkerPosition position, bool center)
    : marker(marker)
    , baseWidth(baseWidth)
    , position(position)
    , center(center)
    {}

    QExplicitlySharedDataPointer<KoMarker> marker;
    qreal baseWidth;
    MarkerPosition position;
    bool center;
};

KoMarkerData::KoMarkerData(KoMarker *marker, qreal width, MarkerPosition position, bool center)
: d(new Private(marker, width, position, center))
{
}

KoMarkerData::KoMarkerData(MarkerPosition position)
: d(new Private(0, 0, position, false))
{
}

KoMarkerData::KoMarkerData()
: d(0)
{
    Q_ASSERT(0);
}

KoMarkerData::KoMarkerData(const KoMarkerData &other)
: d(new Private(other.d->marker.data(), other.d->baseWidth, other.d->position, other.d->center))
{
}

KoMarkerData::~KoMarkerData()
{
    delete d;
}


KoMarker *KoMarkerData::marker() const
{
    return d->marker.data();
}

void KoMarkerData::setMarker(KoMarker *marker)
{
    d->marker = QExplicitlySharedDataPointer<KoMarker>(marker);
}

qreal KoMarkerData::width(qreal penWidth) const
{
    return d->baseWidth + penWidth * ResizeFactor;
}

void KoMarkerData::setWidth(qreal width, qreal penWidth)
{
    d->baseWidth = qMax(qreal(0.0), width - penWidth * ResizeFactor);
}

KoMarkerData::MarkerPosition KoMarkerData::position() const
{
    return d->position;
}

void KoMarkerData::setPosition(MarkerPosition position)
{
    d->position = position;
}

bool KoMarkerData::center() const
{
    return d->center;
}

void KoMarkerData::setCenter(bool center)
{
    d->center = center;
}

KoMarkerData &KoMarkerData::operator=(const KoMarkerData &other)
{
    if (this != &other) {
        d->marker = other.d->marker;
        d->baseWidth = other.d->baseWidth;
        d->position = other.d->position;
        d->center = other.d->center;
    }
    return (*this);
}

bool KoMarkerData::loadOdf(qreal penWidth, KoShapeLoadingContext &context)
{
    KoMarkerSharedLoadingData *markerShared = dynamic_cast<KoMarkerSharedLoadingData*>(context.sharedData(MARKER_SHARED_LOADING_ID));
    if (markerShared) {
        KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
        // draw:marker-end="Arrow" draw:marker-end-width="0.686cm" draw:marker-end-center="true"
        const QString markerStart(styleStack.property(KoXmlNS::draw, markerOdfData[d->position].m_markerPositionLoad));
        const QString markerStartWidth(styleStack.property(KoXmlNS::draw, markerOdfData[d->position].m_markerWidthLoad));
        if (!markerStart.isEmpty() && !markerStartWidth.isEmpty()) {
            KoMarker *marker = markerShared->marker(markerStart);
            if (marker) {
                setMarker(marker);
                qreal markerWidth = KoUnit::parseValue(markerStartWidth);
                setWidth(markerWidth, penWidth);
                setCenter(styleStack.property(KoXmlNS::draw, markerOdfData[d->position].m_markerCenterLoad) == "true");
            }
        }
    }
    return true;
}

void KoMarkerData::saveStyle(KoGenStyle &style, qreal penWidth, KoShapeSavingContext &context) const
{
    if (d->marker) {
        QString markerRef = d->marker->saveOdf(context);
        style.addProperty(markerOdfData[d->position].m_markerPositionSave, markerRef, KoGenStyle::GraphicType);
        style.addPropertyPt(markerOdfData[d->position].m_markerWidthSave, width(penWidth), KoGenStyle::GraphicType);
        style.addProperty(markerOdfData[d->position].m_markerCenterSave, d->center, KoGenStyle::GraphicType);
    }
}
