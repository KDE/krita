/*
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_cubic_curve.h"

#include <QPointF>
#include <QList>
#include <QSharedData>
#include <QStringList>
#include "kis_dom_utils.h"
#include "kis_algebra_2d.h"

KisCubicCurvePoint::KisCubicCurvePoint(const QPointF &position, bool setAsCorner)
    : m_position(position), m_isCorner(setAsCorner)
{}

KisCubicCurvePoint::KisCubicCurvePoint(qreal x, qreal y, bool setAsCorner)
    : m_position(x, y), m_isCorner(setAsCorner)
{}

bool KisCubicCurvePoint::operator==(const KisCubicCurvePoint &other) const
{
    return m_position == other.m_position && m_isCorner == other.m_isCorner;
}

qreal KisCubicCurvePoint::x() const
{
    return m_position.x();
}

qreal KisCubicCurvePoint::y() const
{
    return m_position.y();
}

const QPointF& KisCubicCurvePoint::position() const
{
    return m_position;
}

bool KisCubicCurvePoint::isSetAsCorner() const
{
    return m_isCorner;
}

void KisCubicCurvePoint::setX(qreal newX)
{
    m_position.setX(newX);
}

void KisCubicCurvePoint::setY(qreal newY)
{
    m_position.setY(newY);
}

void KisCubicCurvePoint::setPosition(const QPointF &newPosition)
{
    m_position = newPosition;
}

void KisCubicCurvePoint::setAsCorner(bool newIsSetAsCorner)
{
    m_isCorner = newIsSetAsCorner;
}

static bool pointLessThan(const KisCubicCurvePoint &a, const KisCubicCurvePoint &b)
{
    return a.x() < b.x();
}

struct Q_DECL_HIDDEN KisCubicCurve::Data : public QSharedData {
    Data() {
    }
    Data(const Data& data) : QSharedData() {
        points = data.points;
        name = data.name;
    }
    ~Data() {
    }

    mutable QString name;
    mutable KisCubicSpline<KisCubicCurvePoint, qreal> spline;
    QList<KisCubicCurvePoint> points;
    mutable bool validSpline {false};
    mutable QVector<quint8> u8Transfer;
    mutable bool validU8Transfer {false};
    mutable QVector<quint16> u16Transfer;
    mutable bool validU16Transfer {false};
    mutable QVector<qreal> fTransfer;
    mutable bool validFTransfer {false};

    void updateSpline();
    void keepSorted();
    qreal value(qreal x);
    void invalidate();

    template<typename _T_, typename _T2_>
    void updateTransfer(QVector<_T_>* transfer, bool& valid, _T2_ min, _T2_ max, int size);
};

void KisCubicCurve::Data::updateSpline()
{
    if (validSpline) return;
    validSpline = true;
    spline.createSpline(points);
}

void KisCubicCurve::Data::invalidate()
{
    validSpline = false;
    validFTransfer = false;
    validU16Transfer = false;
}

void KisCubicCurve::Data::keepSorted()
{
    std::sort(points.begin(), points.end(), pointLessThan);
}

qreal KisCubicCurve::Data::value(qreal x)
{
    updateSpline();
    /* Automatically extend non-existing parts of the curve
     * (e.g. before the first point) and cut off big y-values
     */
    x = qBound(points.first().x(), x, points.last().x());
    qreal y = spline.getValue(x);
    return qBound(qreal(0.0), y, qreal(1.0));
}

template<typename _T_, typename _T2_>
void KisCubicCurve::Data::updateTransfer(QVector<_T_>* transfer, bool& valid, _T2_ min, _T2_ max, int size)
{
    if (!valid || transfer->size() != size) {
        if (transfer->size() != size) {
            transfer->resize(size);
        }
        qreal end = 1.0 / (size - 1);
        for (int i = 0; i < size; ++i) {
            /* Direct uncached version */
            _T2_ val = value(i * end ) * max;
            val = qBound(min, val, max);
            (*transfer)[i] = val;
        }
        valid = true;
    }
}

struct Q_DECL_HIDDEN KisCubicCurve::Private {
    QSharedDataPointer<Data> data;
};

KisCubicCurve::KisCubicCurve()
    : d(new Private)
{
    d->data = new Data;
    d->data->points.append({ 0.0, 0.0, false });
    d->data->points.append({ 1.0, 1.0, false });
}

KisCubicCurve::KisCubicCurve(const QList<QPointF> &points)
    : d(new Private)
{
    d->data = new Data;
    Q_FOREACH(const QPointF p, points) {
        d->data->points.append({ p, false });
    }
    d->data->keepSorted();
}

KisCubicCurve::KisCubicCurve(const QList<KisCubicCurvePoint> &points)
    : d(new Private)
{
    d->data = new Data;
    d->data->points = points;
    d->data->keepSorted();
}

KisCubicCurve::KisCubicCurve(const QVector<QPointF> &points)
    : KisCubicCurve(points.toList())
{}

KisCubicCurve::KisCubicCurve(const QVector<KisCubicCurvePoint> &points)
    : KisCubicCurve(points.toList())
{}

KisCubicCurve::KisCubicCurve(const KisCubicCurve& curve)
    : d(new Private(*curve.d))
{
}

KisCubicCurve::KisCubicCurve(const QString &curveString)
    : d(new Private)
{
    // Curve string format: a semi-colon separated list of point entries.
    // Previous point entry format: a pair of numbers, separated by a comma,
    //      that represent the x and y coordinates of the point, in that order.
    //      Examples: identity "0.0,0.0;1.0,1.0;"
    //                U-shaped curve "0.0,1.0;0.5,0.0;1.0,1.0"
    // New point entry format: a list of comma separated point properties. The
    //      first and second properties are required and should be numbers
    //      that represent the x and y coordinates of the point, in that order.
    //      After those, some optional properties/flags may be present or not.
    //      If there are no optional properties for any point entry, then the
    //      format of the string is identical to the old one.
    //      Currently, only the "is_corner" flag is supported. All other
    //      present optional properties are ignored.
    //      Examples: identity "0.0,0.0;1.0,1.0;"
    //                V-shaped curve "0.0,1.0;0.5,0.0,is_corner;1.0,1.0"

    d->data = new Data;

    KIS_SAFE_ASSERT_RECOVER(!curveString.isEmpty()) {
        *this = KisCubicCurve();
        return;
    }

    const QStringList data = curveString.split(';', Qt::SkipEmptyParts);

    QList<KisCubicCurvePoint> points;
    Q_FOREACH (const QString &entry, data) {
        const QStringList entryData = entry.split(',', Qt::SkipEmptyParts);
        KIS_SAFE_ASSERT_RECOVER(entryData.size() > 1) {
            *this = KisCubicCurve();
            return;
        }
        bool ok;
        const qreal x = KisDomUtils::toDouble(entryData[0], &ok);
        KIS_SAFE_ASSERT_RECOVER(ok) {
            *this = KisCubicCurve();
            return;
        }
        const qreal y = KisDomUtils::toDouble(entryData[1], &ok);
        KIS_SAFE_ASSERT_RECOVER(ok) {
            *this = KisCubicCurve();
            return;
        }
        bool isCorner = false;
        for (int i = 2; i < entryData.size(); ++i) {
            if (entryData[i] == "is_corner") {
                isCorner = true;
            }
        }
        points.append({ x, y, isCorner });
    }

    setPoints(points);
}

KisCubicCurve::~KisCubicCurve()
{
    delete d;
}

KisCubicCurve& KisCubicCurve::operator=(const KisCubicCurve & curve)
{
    if (&curve != this) {
        *d = *curve.d;
    }
    return *this;
}

bool KisCubicCurve::operator==(const KisCubicCurve& curve) const
{
    if (d->data == curve.d->data) return true;
    return d->data->points == curve.d->data->points;
}

qreal KisCubicCurve::value(qreal x) const
{
    qreal value = d->data->value(x);
    return value;
}

QList<QPointF> KisCubicCurve::points() const
{
    QList<QPointF> pointPositions;
    Q_FOREACH(const KisCubicCurvePoint &point, d->data->points) {
        pointPositions.append(point.position());
    }
    return pointPositions;
}

const QList<KisCubicCurvePoint>& KisCubicCurve::curvePoints() const
{
    return d->data->points;
}

void KisCubicCurve::setPoints(const QList<QPointF>& points)
{
    d->data.detach();
    d->data->points.clear();
    Q_FOREACH(const QPointF &p, points) {
        d->data->points.append({ p, false });
    }
    d->data->invalidate();
}

void KisCubicCurve::setPoints(const QList<KisCubicCurvePoint>& points)
{
    d->data.detach();
    d->data->points = points;
    d->data->invalidate();
}

void KisCubicCurve::setPoint(int idx, const KisCubicCurvePoint& point)
{
    d->data.detach();
    d->data->points[idx] = point;
    d->data->keepSorted();
    d->data->invalidate();
}

void KisCubicCurve::setPoint(int idx, const QPointF& position, bool setAsCorner)
{
    setPoint(idx, { position, setAsCorner });
}

void KisCubicCurve::setPoint(int idx, const QPointF& position)
{
    setPointPosition(idx, position);
}

void KisCubicCurve::setPointPosition(int idx, const QPointF& position)
{
    d->data.detach();
    d->data->points[idx].setPosition(position);
    d->data->keepSorted();
    d->data->invalidate();
}

void KisCubicCurve::setPointAsCorner(int idx, bool setAsCorner)
{
    d->data.detach();
    d->data->points[idx].setAsCorner(setAsCorner);
    d->data->invalidate();
}

int KisCubicCurve::addPoint(const KisCubicCurvePoint& point)
{
    d->data.detach();
    d->data->points.append(point);
    d->data->keepSorted();
    d->data->invalidate();

    return d->data->points.indexOf(point);
}

int KisCubicCurve::addPoint(const QPointF& position, bool setAsCorner)
{
    return addPoint({ position, setAsCorner });
}

int KisCubicCurve::addPoint(const QPointF& position)
{
    return addPoint({ position, false });
}

void KisCubicCurve::removePoint(int idx)
{
    d->data.detach();
    d->data->points.removeAt(idx);
    d->data->invalidate();
}

bool KisCubicCurve::isIdentity() const
{
    const QList<KisCubicCurvePoint> &points = d->data->points;

    if (points.first().x() != 0.0 || points.first().y() != 0.0 ||
        points.last().x() != 1.0 || points.last().y() != 1.0) {
        return false;
    }

    for (int i = 1; i < points.size() - 1; i++) {
        if (!qFuzzyCompare(points[i].x(), points[i].y())) {
            return false;
        }
    }

    return true;
}

bool KisCubicCurve::isConstant(qreal c) const
{
    const QList<KisCubicCurvePoint> &points = d->data->points;

    Q_FOREACH (const KisCubicCurvePoint &pt, points) {
        if (!qFuzzyCompare(c, pt.y())) {
            return false;
        }
    }

    return true;
}

const QString& KisCubicCurve::name() const
{
    return d->data->name;
}

qreal KisCubicCurve::interpolateLinear(qreal normalizedValue, const QVector<qreal> &transfer)
{
    const qreal maxValue = transfer.size() - 1;

    const qreal bilinearX = qBound(0.0, maxValue * normalizedValue, maxValue);
    const qreal xFloored = std::floor(bilinearX);
    const qreal xCeiled = std::ceil(bilinearX);

    const qreal t = bilinearX - xFloored;

    constexpr qreal eps = 1e-6;

    qreal newValue = normalizedValue;

    if (t < eps) {
        newValue = transfer[int(xFloored)];
    } else if (t > (1.0 - eps)) {
        newValue = transfer[int(xCeiled)];
    } else {
        qreal a = transfer[int(xFloored)];
        qreal b = transfer[int(xCeiled)];

        newValue = a + t * (b - a);
    }

    return KisAlgebra2D::copysign(newValue, normalizedValue);
}

void KisCubicCurve::setName(const QString& name)
{
    d->data->name = name;
}

QString KisCubicCurve::toString() const
{
    // See comments in KisCubicCurve(const QString &curveString) for the
    // specification of the string format
    
    QString sCurve;

    if(d->data->points.count() < 1)
        return sCurve;

    Q_FOREACH (const KisCubicCurvePoint &point, d->data->points) {
        sCurve += QString::number(point.x());
        sCurve += ',';
        sCurve += QString::number(point.y());
        if (point.isSetAsCorner()) {
            sCurve += ',';
            sCurve += "is_corner";
        }
        sCurve += ';';
    }

    return sCurve;
}

void KisCubicCurve::fromString(const QString& string)
{
    *this = KisCubicCurve(string);
}

const QVector<quint16> KisCubicCurve::uint16Transfer(int size) const
{
    d->data->updateTransfer<quint16, int>(&d->data->u16Transfer, d->data->validU16Transfer, 0x0, 0xFFFF, size);
    return d->data->u16Transfer;
}

const QVector<qreal> KisCubicCurve::floatTransfer(int size) const
{
    d->data->updateTransfer<qreal, qreal>(&d->data->fTransfer, d->data->validFTransfer, 0.0, 1.0, size);
    return d->data->fTransfer;
}
