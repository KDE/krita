/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Laurent Montel <montel@kde.org>
   SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kis_guides_config.h"

#include <QDomDocument>
#include <QColor>
#include <QPen>

#include "kis_config.h"
#include "kis_dom_utils.h"
#include "kis_algebra_2d.h"
#include "kis_global.h"


struct KisGuidesConfigStaticRegistrar {
    KisGuidesConfigStaticRegistrar() {
        qRegisterMetaType<KisGuidesConfig>("KisGuidesConfig");
    }
};
static KisGuidesConfigStaticRegistrar __registrar;


class Q_DECL_HIDDEN KisGuidesConfig::Private
{
public:
    Private()
        : showGuides(false)
        , snapToGuides(false)
        , lockGuides(false)
        , rulersMultiple2(false)
        , unitType(KoUnit::Pixel)
    {}

    bool operator==(const Private &rhs) {
        return horzGuideLines == rhs.horzGuideLines &&
            vertGuideLines == rhs.vertGuideLines &&
            showGuides == rhs.showGuides &&
            snapToGuides == rhs.snapToGuides &&
            lockGuides == rhs.lockGuides &&
            guidesColor == rhs.guidesColor &&
            guidesLineType == rhs.guidesLineType &&
            rulersMultiple2 == rhs.rulersMultiple2 &&
            unitType == rhs.unitType;
    }

    QList<qreal> horzGuideLines;
    QList<qreal> vertGuideLines;

    bool showGuides;
    bool snapToGuides;
    bool lockGuides;
    bool rulersMultiple2;

    KoUnit::Type unitType;

    QColor guidesColor;
    LineTypeInternal guidesLineType;

    Qt::PenStyle toPenStyle(LineTypeInternal type);
};

KisGuidesConfig::KisGuidesConfig()
    : d(new Private())
{
    loadStaticData();
}

KisGuidesConfig::~KisGuidesConfig()
{
}

KisGuidesConfig::KisGuidesConfig(const KisGuidesConfig &rhs)
    : d(new Private(*rhs.d))
{
}

KisGuidesConfig& KisGuidesConfig::operator=(const KisGuidesConfig &rhs)
{
    if (&rhs != this) {
        *d = *rhs.d;
    }

    return *this;
}

bool KisGuidesConfig::operator==(const KisGuidesConfig &rhs) const
{
    return *d == *rhs.d;
}

bool KisGuidesConfig::hasSamePositionAs(const KisGuidesConfig &rhs) const
{
    return horizontalGuideLines() == rhs.horizontalGuideLines() &&
        verticalGuideLines() == rhs.verticalGuideLines();
}

void KisGuidesConfig::setHorizontalGuideLines(const QList<qreal> &lines)
{
    d->horzGuideLines = lines;
}

void KisGuidesConfig::setVerticalGuideLines(const QList<qreal> &lines)
{
    d->vertGuideLines = lines;
}

void KisGuidesConfig::addGuideLine(Qt::Orientation o, qreal pos)
{
    if (o == Qt::Horizontal) {
        d->horzGuideLines.append(pos);
    } else {
        d->vertGuideLines.append(pos);
    }
}

bool KisGuidesConfig::showGuideLines() const
{
    return d->showGuides;
}

void KisGuidesConfig::setShowGuideLines(bool show)
{
    d->showGuides = show;
}

bool KisGuidesConfig::showGuides() const
{
    return d->showGuides;
}

void KisGuidesConfig::setShowGuides(bool value)
{
    d->showGuides = value;
}

bool KisGuidesConfig::lockGuides() const
{
    return d->lockGuides;
}

void KisGuidesConfig::setLockGuides(bool value)
{
    d->lockGuides = value;
}

bool KisGuidesConfig::snapToGuides() const
{
    return d->snapToGuides;
}

void KisGuidesConfig::setSnapToGuides(bool value)
{
    d->snapToGuides = value;
}

bool KisGuidesConfig::rulersMultiple2() const
{
    return d->rulersMultiple2;
}

void KisGuidesConfig::setRulersMultiple2(bool value)
{
    d->rulersMultiple2 = value;
}

KoUnit::Type KisGuidesConfig::unitType() const
{
    return d->unitType;
}

void KisGuidesConfig::setUnitType(const KoUnit::Type type)
{
    d->unitType = type;
}

KisGuidesConfig::LineTypeInternal
KisGuidesConfig::guidesLineType() const
{
    return d->guidesLineType;
}

void KisGuidesConfig::setGuidesLineType(LineTypeInternal value)
{
    d->guidesLineType = value;
}


QColor KisGuidesConfig::guidesColor() const
{
    return d->guidesColor;
}

void KisGuidesConfig::setGuidesColor(const QColor &value)
{
    d->guidesColor = value;
}

 Qt::PenStyle KisGuidesConfig::Private::toPenStyle(LineTypeInternal type) {
    return type == LINE_SOLID ? Qt::SolidLine :
        type == LINE_DASHED ? Qt::DashLine :
        type == LINE_DOTTED ? Qt::DotLine :
        Qt::DashDotDotLine;
}

QPen KisGuidesConfig::guidesPen() const
{
    return QPen(d->guidesColor, 0, d->toPenStyle(d->guidesLineType));
}

const QList<qreal>& KisGuidesConfig::horizontalGuideLines() const
{
    return d->horzGuideLines;
}

const QList<qreal>& KisGuidesConfig::verticalGuideLines() const
{
    return d->vertGuideLines;
}

bool KisGuidesConfig::hasGuides() const
{
    return !d->horzGuideLines.isEmpty() || !d->vertGuideLines.isEmpty();
}

void KisGuidesConfig::loadStaticData()
{
    KisConfig cfg(true);
    d->guidesLineType = LineTypeInternal(cfg.guidesLineStyle());
    d->guidesColor = cfg.guidesColor();
}

void KisGuidesConfig::saveStaticData() const
{
    KisConfig cfg(false);
    cfg.setGuidesLineStyle(d->guidesLineType);
    cfg.setGuidesColor(d->guidesColor);
}

QDomElement KisGuidesConfig::saveToXml(QDomDocument& doc, const QString &tag) const
{
    QDomElement guidesElement = doc.createElement(tag);
    KisDomUtils::saveValue(&guidesElement, "showGuides", d->showGuides);
    KisDomUtils::saveValue(&guidesElement, "snapToGuides", d->snapToGuides);
    KisDomUtils::saveValue(&guidesElement, "lockGuides", d->lockGuides);

    KisDomUtils::saveValue(&guidesElement, "horizontalGuides", d->horzGuideLines.toVector());
    KisDomUtils::saveValue(&guidesElement, "verticalGuides", d->vertGuideLines.toVector());

    KisDomUtils::saveValue(&guidesElement, "rulersMultiple2", d->rulersMultiple2);
    KoUnit tmp(d->unitType);
    KisDomUtils::saveValue(&guidesElement, "unit", tmp.symbol());

    return guidesElement;
}

bool KisGuidesConfig::loadFromXml(const QDomElement &parent)
{
    bool result = true;

    result &= KisDomUtils::loadValue(parent, "showGuides", &d->showGuides);
    result &= KisDomUtils::loadValue(parent, "snapToGuides", &d->snapToGuides);
    result &= KisDomUtils::loadValue(parent, "lockGuides", &d->lockGuides);

    QVector<qreal> hGuides;
    QVector<qreal> vGuides;

    result &= KisDomUtils::loadValue(parent, "horizontalGuides", &hGuides);
    result &= KisDomUtils::loadValue(parent, "verticalGuides", &vGuides);

    d->horzGuideLines = QList<qreal>::fromVector(hGuides);
    d->vertGuideLines = QList<qreal>::fromVector(vGuides);

    result &= KisDomUtils::loadValue(parent, "rulersMultiple2", &d->rulersMultiple2);
    QString unit;
    result &= KisDomUtils::loadValue(parent, "unit", &unit);
    bool ok = false;
    KoUnit tmp = KoUnit::fromSymbol(unit, &ok);
    if (ok) {
        d->unitType = tmp.type();
    }
    result &= ok;


    return result;
}

bool KisGuidesConfig::isDefault() const
{
    KisGuidesConfig defaultObject;
    defaultObject.loadStaticData();

    return *this == defaultObject;
}

void KisGuidesConfig::transform(const QTransform &transform)
{
    if (transform.type() >= QTransform::TxShear) return;

    KisAlgebra2D::DecomposedMatix m(transform);

    QTransform t = m.scaleTransform();

    const qreal eps = 1e-3;
    int numWraps = 0;
    const qreal wrappedRotation = KisAlgebra2D::wrapValue(m.angle, 90.0);
    if (wrappedRotation <= eps || wrappedRotation >= 90.0 - eps) {
        t *= m.rotateTransform();
        numWraps = qRound(normalizeAngleDegrees(m.angle) / 90.0);
    }

    t *= m.translateTransform();


    QList<qreal> newHorzGuideLines;
    QList<qreal> newVertGuideLines;

    Q_FOREACH (qreal hRuler, d->horzGuideLines) {
        const QPointF pt = t.map(QPointF(0, hRuler));

        if (numWraps & 0x1) {
            newVertGuideLines << pt.x();
        } else {
            newHorzGuideLines << pt.y();
        }
    }

    Q_FOREACH (qreal vRuler, d->vertGuideLines) {
        const QPointF pt = t.map(QPointF(vRuler, 0));

        if (!(numWraps & 0x1)) {
            newVertGuideLines << pt.x();
        } else {
            newHorzGuideLines << pt.y();
        }
    }

    d->horzGuideLines = newHorzGuideLines;
    d->vertGuideLines = newVertGuideLines;
}

bool KisGuidesConfig::isSameIgnoringSnapping(const KisGuidesConfig &rhs) const
{
    KisGuidesConfig tmp(rhs);
    tmp.setSnapToGuides(snapToGuides());

    return *this == tmp;
}
