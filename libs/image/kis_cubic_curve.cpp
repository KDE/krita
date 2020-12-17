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

template <typename T>
class KisTridiagonalSystem
{
    /*
     * e.g.
     *      |b0 c0  0   0   0| |x0| |f0|
     *      |a0 b1 c1   0   0| |x1| |f1|
     *      |0  a1 b2  c2   0|*|x2|=|f2|
     *      |0   0 a2  b3  c3| |x3| |f3|
     *      |0   0  0  a3  b4| |x4| |f4|
     */

public:

    /**
     * @return - vector that is storing x[]
     */
    static
    QVector<T> calculate(QList<T> &a,
                         QList<T> &b,
                         QList<T> &c,
                         QList<T> &f) {
        QVector<T> x;
        QVector<T> alpha;
        QVector<T> beta;

        int i;
        int size = b.size();

        Q_ASSERT(a.size() == size - 1 &&
                 c.size() == size - 1 &&
                 f.size() == size);

        x.resize(size);

        /**
         * Check for special case when
         * order of the matrix is equal to 1
         */
        if (size == 1) {
            x[0] = f[0] / b[0];
            return x;
        }

        /**
         * Common case
         */

        alpha.resize(size);
        beta.resize(size);


        alpha[1] = -c[0] / b[0];
        beta[1] =  f[0] / b[0];

        for (i = 1; i < size - 1; i++) {
            alpha[i+1] = -c[i] /
                         (a[i-1] * alpha[i] + b[i]);

            beta[i+1] = (f[i] - a[i-1] * beta[i])
                        /
                        (a[i-1] * alpha[i] + b[i]);
        }

        x.last() = (f.last() - a.last() * beta.last())
                   /
                   (b.last() + a.last() * alpha.last());

        for (i = size - 2; i >= 0; i--)
            x[i] = alpha[i+1] * x[i+1] + beta[i+1];

        return x;
    }
};

template <typename T_point, typename T>
class KisCubicSpline
{
    /**
     *  s[i](x)=a[i] +
     *          b[i] * (x-x[i]) +
     *    1/2 * c[i] * (x-x[i])^2 +
     *    1/6 * d[i] * (x-x[i])^3
     *
     *  h[i]=x[i+1]-x[i]
     *
     */

protected:
    QList<T> m_a;
    QVector<T> m_b;
    QVector<T> m_c;
    QVector<T> m_d;

    QVector<T> m_h;
    T m_begin;
    T m_end;
    int m_intervals;

public:
    KisCubicSpline() {}
    KisCubicSpline(const QList<T_point> &a) {
        createSpline(a);
    }

    /**
     * Create new spline and precalculate some values
     * for future
     *
     * @a - base points of the spline
     */
    void createSpline(const QList<T_point> &a) {
        int intervals = m_intervals = a.size() - 1;
        int i;
        m_begin = a.first().x();
        m_end = a.last().x();

        m_a.clear();
        m_b.resize(intervals);
        m_c.clear();
        m_d.resize(intervals);
        m_h.resize(intervals);

        for (i = 0; i < intervals; i++) {
            m_h[i] = a[i+1].x() - a[i].x();
            m_a.append(a[i].y());
        }
        m_a.append(a.last().y());


        QList<T> tri_b;
        QList<T> tri_f;
        QList<T> tri_a; /* equals to @tri_c */

        for (i = 0; i < intervals - 1; i++) {
            tri_b.append(2.*(m_h[i] + m_h[i+1]));

            tri_f.append(6.*((m_a[i+2] - m_a[i+1]) / m_h[i+1] - (m_a[i+1] - m_a[i]) / m_h[i]));
        }
        for (i = 1; i < intervals - 1; i++)
            tri_a.append(m_h[i]);

        if (intervals > 1) {
            m_c = KisTridiagonalSystem<T>::calculate(tri_a, tri_b, tri_a, tri_f);
        }
        m_c.prepend(0);
        m_c.append(0);

        for (i = 0; i < intervals; i++)
            m_d[i] = (m_c[i+1] - m_c[i]) / m_h[i];

        for (i = 0; i < intervals; i++)
            m_b[i] = -0.5 * (m_c[i] * m_h[i])  - (1 / 6.0) * (m_d[i] * m_h[i] * m_h[i]) + (m_a[i+1] - m_a[i]) / m_h[i];
    }

    /**
     * Get value of precalculated spline in the point @x
     */
    T getValue(T x) const {
        T x0;
        int i = findRegion(x, x0);
        /* TODO: check for asm equivalent */
        return m_a[i] +
               m_b[i] *(x - x0) +
               0.5 * m_c[i] *(x - x0) *(x - x0) +
               (1 / 6.0)* m_d[i] *(x - x0) *(x - x0) *(x - x0);
    }

    T begin() const {
        return m_begin;
    }

    T end() const {
        return m_end;
    }

protected:

    /**
     * findRegion - Searches for the region containing @x
     * @x0 - out parameter, containing beginning of the region
     * @return - index of the region
     */
    int findRegion(T x, T &x0) const {
        int i;
        x0 = m_begin;
        for (i = 0; i < m_intervals; i++) {
            if (x >= x0 && x < x0 + m_h[i])
                return i;
            x0 += m_h[i];
        }
        if (x >= x0) {
            x0 -= m_h[m_intervals-1];
            return m_intervals - 1;
        }

        qDebug("X value: %f\n", x);
        qDebug("m_begin: %f\n", m_begin);
        qDebug("m_end  : %f\n", m_end);
        Q_ASSERT_X(0, "findRegion", "X value is outside regions");
        /* **never reached** */
        return -1;
    }
};

static bool pointLessThan(const QPointF &a, const QPointF &b)
{
    return a.x() < b.x();
}

struct Q_DECL_HIDDEN KisCubicCurve::Data : public QSharedData {
    Data() {
        init();
    }
    Data(const Data& data) : QSharedData() {
        init();
        points = data.points;
        name = data.name;
    }
    void init() {
        validSpline = false;
        validU16Transfer = false;
        validFTransfer = false;
    }
    ~Data() {
    }

    mutable QString name;
    mutable KisCubicSpline<QPointF, qreal> spline;
    QList<QPointF> points;
    mutable bool validSpline;
    mutable QVector<quint8> u8Transfer;
    mutable bool validU8Transfer;
    mutable QVector<quint16> u16Transfer;
    mutable bool validU16Transfer;
    mutable QVector<qreal> fTransfer;
    mutable bool validFTransfer;
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
    x = qBound(spline.begin(), x, spline.end());
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

KisCubicCurve::KisCubicCurve() : d(new Private)
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

KisCubicCurve::KisCubicCurve(const KisCubicCurve& curve)
    : d(new Private(*curve.d))
{
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
    QStringList data = string.split(';');

    QList<QPointF> points;

    Q_FOREACH (const QString & pair, data) {
        if (pair.indexOf(',') > -1) {
            QPointF p;
            p.rx() = KisDomUtils::toDouble(pair.section(',', 0, 0));
            p.ry() = KisDomUtils::toDouble(pair.section(',', 1, 1));
            points.append(p);
        }
    }
    setPoints(points);
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
