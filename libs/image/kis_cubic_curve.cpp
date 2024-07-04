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
#include "kis_cubic_curve_spline.h"

static bool pointLessThan(const QPointF &a, const QPointF &b)
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
    mutable KisCubicSpline<QPointF, qreal> spline;
    QList<QPointF> points;
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
    QPointF p;
    p.rx() = 0.0; p.ry() = 0.0;
    d->data->points.append(p);
    p.rx() = 1.0; p.ry() = 1.0;
    d->data->points.append(p);
}

KisCubicCurve::KisCubicCurve(const QList<QPointF>& points) : d(new Private)
{
    d->data = new Data;
    d->data->points = points;
    d->data->keepSorted();
}

KisCubicCurve::KisCubicCurve(const QVector<QPointF> &points)
    : KisCubicCurve(points.toList())
{
}

KisCubicCurve::KisCubicCurve(const KisCubicCurve& curve)
    : d(new Private(*curve.d))
{
}

KisCubicCurve::KisCubicCurve(const QString &curveString)
    : d(new Private)
{
    d->data = new Data;

    KIS_SAFE_ASSERT_RECOVER(!curveString.isEmpty()) {
        *this = KisCubicCurve();
        return;
    }

    const QStringList data = curveString.split(';');

    QList<QPointF> points;
    Q_FOREACH (const QString & pair, data) {
        if (pair.indexOf(',') > -1) {
            QPointF p;
            p.rx() = KisDomUtils::toDouble(pair.section(',', 0, 0));
            p.ry() = KisDomUtils::toDouble(pair.section(',', 1, 1));
            points.append(p);
        }
    }

    d->data->points = points;
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

const QList<QPointF>& KisCubicCurve::points() const
{
    return d->data->points;
}

void KisCubicCurve::setPoints(const QList<QPointF>& points)
{
    d->data.detach();
    d->data->points = points;
    d->data->invalidate();
}

void KisCubicCurve::setPoint(int idx, const QPointF& point)
{
    d->data.detach();
    d->data->points[idx] = point;
    d->data->keepSorted();
    d->data->invalidate();
}

int KisCubicCurve::addPoint(const QPointF& point)
{ 
    d->data.detach();
    d->data->points.append(point);
    d->data->keepSorted();
    d->data->invalidate();

    return d->data->points.indexOf(point);
}

void KisCubicCurve::removePoint(int idx)
{
    d->data.detach();
    d->data->points.removeAt(idx);
    d->data->invalidate();
}

bool KisCubicCurve::isIdentity() const
{
    const QList<QPointF> &points = d->data->points;
    const int size = points.size();

    if (points[0] != QPointF(0,0) || points[size-1] != QPointF(1,1)) {
        return false;
    }

    for (int i = 1; i < size-1; i++) {
        if (!qFuzzyCompare(points[i].x(), points[i].y())) {
            return false;
        }
    }

    return true;
}

bool KisCubicCurve::isConstant(qreal c) const
{
    const QList<QPointF> &points = d->data->points;

    Q_FOREACH (const QPointF &pt, points) {
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
    QString sCurve;

    if(d->data->points.count() < 1)
        return sCurve;

    Q_FOREACH (const QPointF & pair, d->data->points) {
        sCurve += QString::number(pair.x());
        sCurve += ',';
        sCurve += QString::number(pair.y());
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
