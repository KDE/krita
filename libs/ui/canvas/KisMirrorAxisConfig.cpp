/*
 *  Copyright (c) 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kis_dom_utils.h>
#include <QPointF>

#include "KisMirrorAxisConfig.h"


class Q_DECL_HIDDEN KisMirrorAxisConfig::Private
{
public:
    Private()
        : mirrorHorizontal(false)
        , mirrorVertical(false)
        , lockHorizontal(false)
        , lockVertical(false)
        , hideVerticalDecoration(false)
        , hideHorizontalDecoration(false)
        , handleSize(32.f)
        , horizontalHandlePosition(64.f)
        , verticalHandlePosition(64.f)
        , axisPosition(QPointF(0.f,0.f))
    {}

    bool operator==(const Private& rhs) {
        return mirrorHorizontal == rhs.mirrorHorizontal &&
                mirrorVertical == rhs.mirrorVertical &&
                lockHorizontal == rhs.lockHorizontal &&
                lockVertical == rhs.lockVertical &&
                hideHorizontalDecoration == rhs.hideHorizontalDecoration &&
                hideVerticalDecoration == rhs.hideVerticalDecoration &&
                handleSize == rhs.handleSize &&
                horizontalHandlePosition == rhs.horizontalHandlePosition &&
                verticalHandlePosition == rhs.verticalHandlePosition &&
                axisPosition == rhs.axisPosition;
    }

    bool mirrorHorizontal;
    bool mirrorVertical;
    bool lockHorizontal;
    bool lockVertical;
    bool hideVerticalDecoration;
    bool hideHorizontalDecoration;

    float handleSize;
    float horizontalHandlePosition;
    float verticalHandlePosition;

    QPointF axisPosition;
};


KisMirrorAxisConfig::KisMirrorAxisConfig()
    : QObject()
    , d(new Private())
{

}

KisMirrorAxisConfig::~KisMirrorAxisConfig()
{

}

KisMirrorAxisConfig::KisMirrorAxisConfig(const KisMirrorAxisConfig &rhs)
    : QObject()
    , d(new Private(*rhs.d))
{

}

KisMirrorAxisConfig &KisMirrorAxisConfig::operator=(const KisMirrorAxisConfig &rhs)
{
    if (&rhs != this) {
        *d = *rhs.d;
    }

    return *this;
}

bool KisMirrorAxisConfig::operator==(const KisMirrorAxisConfig &rhs) const
{
    KIS_ASSERT(d);
    KIS_ASSERT(rhs.d);

    return *d == *rhs.d;
}

bool KisMirrorAxisConfig::mirrorHorizontal()
{
    return d->mirrorHorizontal;
}

void KisMirrorAxisConfig::setMirrorHorizontal(bool state)
{
    d->mirrorHorizontal = state;
}

bool KisMirrorAxisConfig::mirrorVertical()
{
    return d->mirrorVertical;
}

void KisMirrorAxisConfig::setMirrorVertical(bool state)
{
    d->mirrorVertical = state;
}

bool KisMirrorAxisConfig::lockHorizontal()
{
    return d->lockHorizontal;
}

void KisMirrorAxisConfig::setLockHorizontal(bool state)
{
    d->lockHorizontal = state;
}

bool KisMirrorAxisConfig::lockVertical()
{
    return d->lockVertical;
}

void KisMirrorAxisConfig::setLockVertical(bool state)
{
    d->lockVertical = state;
}

bool KisMirrorAxisConfig::hideVerticalDecoration()
{
    return d->hideVerticalDecoration;
}

void KisMirrorAxisConfig::setHideVerticalDecoration(bool state)
{
    d->hideVerticalDecoration = state;
}

bool KisMirrorAxisConfig::hideHorizontalDecoration()
{
    return d->hideHorizontalDecoration;
}

void KisMirrorAxisConfig::setHideHorizontalDecoration(bool state)
{
    d->hideHorizontalDecoration = state;
}

float KisMirrorAxisConfig::handleSize()
{
    return d->handleSize;
}

void KisMirrorAxisConfig::setHandleSize(float size)
{
    d->handleSize = size;
}

float KisMirrorAxisConfig::horizontalHandlePosition()
{
    return d->horizontalHandlePosition;
}

void KisMirrorAxisConfig::setHorizontalHandlePosition(float position)
{
    d->horizontalHandlePosition = position;
}

float KisMirrorAxisConfig::verticalHandlePosition()
{
    return d->verticalHandlePosition;
}

void KisMirrorAxisConfig::setVerticalHandlePosition(float position)
{
    d->verticalHandlePosition = position;
}

QPointF KisMirrorAxisConfig::axisPosition()
{
    return d->axisPosition;
}

void KisMirrorAxisConfig::setAxisPosition(QPointF position)
{
    d->axisPosition = position;
}

QDomElement KisMirrorAxisConfig::saveToXml(QDomDocument &doc, const QString &tag) const
{
    QDomElement mirrorAxisElement = doc.createElement(tag);
    KisDomUtils::saveValue(&mirrorAxisElement, "mirrorHorizontal", d->mirrorHorizontal);
    KisDomUtils::saveValue(&mirrorAxisElement, "mirrorVertical", d->mirrorVertical);
    KisDomUtils::saveValue(&mirrorAxisElement, "lockHorizontal", d->lockHorizontal);
    KisDomUtils::saveValue(&mirrorAxisElement, "lockVertical", d->lockVertical);

    KisDomUtils::saveValue(&mirrorAxisElement, "hideHorizontalDecoration", d->hideHorizontalDecoration);
    KisDomUtils::saveValue(&mirrorAxisElement, "hideVerticalDecoration", d->hideVerticalDecoration);

    KisDomUtils::saveValue(&mirrorAxisElement, "handleSize", d->handleSize);

    KisDomUtils::saveValue(&mirrorAxisElement, "horizontalHandlePosition", d->horizontalHandlePosition);
    KisDomUtils::saveValue(&mirrorAxisElement, "verticalHandlePosition", d->verticalHandlePosition);

    KisDomUtils::saveValue(&mirrorAxisElement, "axisPosition", d->axisPosition);

    return mirrorAxisElement;
}

bool KisMirrorAxisConfig::loadFromXml(const QDomElement &parent)
{
    bool result = true;

    result &= KisDomUtils::loadValue(parent, "mirrorHorizontal", &d->mirrorHorizontal);
    result &= KisDomUtils::loadValue(parent, "mirrorVertical", &d->mirrorVertical);
    result &= KisDomUtils::loadValue(parent, "lockHorizontal", &d->lockHorizontal);
    result &= KisDomUtils::loadValue(parent, "lockVertical", &d->lockVertical);

    result &= KisDomUtils::loadValue(parent, "hideHorizontalDecoration", &d->hideHorizontalDecoration);
    result &= KisDomUtils::loadValue(parent, "hideVerticalDecoration", &d->hideVerticalDecoration);

    result &= KisDomUtils::loadValue(parent, "handleSize", &d->handleSize);

    result &= KisDomUtils::loadValue(parent, "horizontalHandlePosition", &d->horizontalHandlePosition);
    result &= KisDomUtils::loadValue(parent, "verticalHandlePosition", &d->verticalHandlePosition);
    result &= KisDomUtils::loadValue(parent, "axisPosition", &d->axisPosition);

    return result;
}

bool KisMirrorAxisConfig::isDefault() const
{
    KisMirrorAxisConfig defaultConfig;
    return *this == defaultConfig;
}
