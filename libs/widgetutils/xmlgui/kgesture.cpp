/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Andreas Hartmetz (ahartmetz@gmail.com)

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kgesture_p.h"
#include <klocalizedstring.h>
#include <math.h>
#include <QStringList>

inline float metric(float dx, float dy)
{
    //square root of that or not? - not square root has possible advantages
    return (dx * dx + dy * dy);
}

class KisKShapeGesturePrivate
{
public:
    KisKShapeGesturePrivate()
    {
    }
    KisKShapeGesturePrivate(const KisKShapeGesturePrivate &other)
        : m_shape(other.m_shape),
          m_lengthTo(other.m_lengthTo),
          m_curveLength(other.m_curveLength)
    {
    }
    QPolygon m_shape;
    QVector<float> m_lengthTo;
    float m_curveLength {0.0};
    QString m_friendlyName;
};

KisKShapeGesture::KisKShapeGesture()
    : d(new KisKShapeGesturePrivate)
{
}

KisKShapeGesture::KisKShapeGesture(const QPolygon &shape)
    : d(new KisKShapeGesturePrivate)
{
    setShape(shape);
}

KisKShapeGesture::KisKShapeGesture(const QString &description)
    : d(new KisKShapeGesturePrivate)
{
    QStringList sl = description.split(QLatin1Char(','));
    d->m_friendlyName = sl.takeFirst();

    bool ok = true;
    QPolygon poly;
    int x, y;
    QStringList::const_iterator it = sl.constBegin();
    while (it != sl.constEnd()) {
        x = (*it).toInt(&ok);
        ++it;
        if (!ok || it == sl.constEnd()) {
            break;
        }
        y = (*it).toInt(&ok);
        if (!ok) {
            break;
        }
        ++it;
        poly.append(QPoint(x, y));
    }
    if (!ok) {
        d->m_friendlyName.clear();
        return;
    }

    setShape(poly);
}

KisKShapeGesture::KisKShapeGesture(const KisKShapeGesture &other)
    : d(new KisKShapeGesturePrivate(*(other.d)))
{
}

KisKShapeGesture::~KisKShapeGesture()
{
    delete d;
}

void KisKShapeGesture::setShape(const QPolygon &shape)
{
    //Scale and translate into a 100x100 square with its
    //upper left corner at origin.
    d->m_shape = shape;
    QRect bounding = shape.boundingRect();
    //TODO: don't change aspect ratio "too much" to avoid problems with straight lines
    //TODO: catch all bad input, like null height/width

    //compensate for QRect weirdness
    bounding.setWidth(bounding.width() - 1);
    bounding.setHeight(bounding.height() - 1);

    float xScale = bounding.width() ? 100.0 / bounding.width() : 1.0;
    float yScale = bounding.height() ? 100.0 / bounding.height() : 1.0;
    d->m_shape.translate(-bounding.left(), -bounding.top());
    for (int i = 0; i < d->m_shape.size(); i++) {
        d->m_shape[i].setX((int)(xScale * (float)d->m_shape[i].x()));
        d->m_shape[i].setY((int)(yScale * (float)d->m_shape[i].y()));
    }

    //calculate accumulated lengths of lines making up the polygon
    Q_ASSERT(d->m_shape.size() > 1);
    d->m_curveLength = 0.0;
    d->m_lengthTo.clear();
    d->m_lengthTo.reserve(d->m_shape.size());
    d->m_lengthTo.append(d->m_curveLength);

    int prevX = d->m_shape[0].x();
    int prevY = d->m_shape[0].y();
    for (int i = 1; i < d->m_shape.size(); i++) {
        int curX = d->m_shape[i].x();
        int curY = d->m_shape[i].y();
        d->m_curveLength += metric(curX - prevX, curY - prevY);
        d->m_lengthTo.append(d->m_curveLength);
        prevX = curX;
        prevY = curY;
    }
}

void KisKShapeGesture::setShapeName(const QString &friendlyName)
{
    d->m_friendlyName = friendlyName;
}

QString KisKShapeGesture::shapeName() const
{
    return d->m_friendlyName;
}

bool KisKShapeGesture::isValid() const
{
    return !d->m_shape.isEmpty();
}

QString KisKShapeGesture::toString() const
{
    if (!isValid()) {
        return QString();
    }

    //TODO: what if the name contains a "," or ";"? Limit the name to letters?
    QString ret = d->m_friendlyName;

    int i;
    for (i = 0; i < d->m_shape.size(); i++) {
        ret.append(QLatin1Char(','));
        ret.append(QString::number(d->m_shape[i].x()));
        ret.append(QLatin1Char(','));
        ret.append(QString::number(d->m_shape[i].y()));
    }

    return ret;
}

QByteArray KisKShapeGesture::toSvg(const QString &attributes) const
{
    if (!isValid()) {
        return QByteArray();
        //TODO: KDE standard debug output
    }
    const char *prolog = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
                         "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
                         "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">"
                         "<svg width=\"100\" height=\"100\" version=\"1.1\" "
                         "xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M";
    const char *epilog1 = "\" fill=\"none\" ";
    const char *epilog2 = " /></svg>";
    QByteArray ret(prolog);

    ret.append(QString::number(d->m_shape[0].x()).toUtf8());
    ret.append(",");
    ret.append(QString::number(d->m_shape[0].y()).toUtf8());

    for (int i = 1; i < d->m_shape.size(); i++) {
        ret.append("L");
        ret.append(QString::number(d->m_shape[i].x()).toUtf8());
        ret.append(",");
        ret.append(QString::number(d->m_shape[i].y()).toUtf8());
    }

    ret.append(epilog1);
    ret.append(attributes.toUtf8());
    ret.append(epilog2);
    return ret;
}

/*
  algorithm: iterate in order over 30 points on our shape and measure the
  minimum distance to any point on the other shape. never go backwards on
  the other shape to also check direction of movement.
  This algorithm is best applied like a->distance(b) + b->distance(a).
  fabs(a->distance(b) - b->distance(a)) might turn out to be very interesting,
  too. in fact, i think it's the most interesting value.
 */
float KisKShapeGesture::distance(const KisKShapeGesture &other, float abortThreshold) const
{
    Q_UNUSED(abortThreshold); //for optimizations, later
    const QPolygon &o_shape = other.d->m_shape;
    const QVector<float> &o_lengthTo = other.d->m_lengthTo;
    float x = 0;
    float y = 0;
    float mx = 0;
    float my = 0;
    float position = 0;
    float ox = 0;
    float oy = 0;
    float opposition = 0;
    float omx = 0;
    float omy = 0;
    float oxB = 0;
    float oyB = 0;
    float oppositionB = 0;
    float omxB = 0;
    float omyB = 0;
    float dist = 0;
    float distB = 0;
    float desiredPosition = 0;
    float strokeLength = 0;
    float retval = 0.0;
    int pointIndex = 0, opointIndex = 0, opointIndexB = 0;

    //set up starting point on our shape
    x = d->m_shape[0].x();
    y = d->m_shape[0].y();
    strokeLength = d->m_lengthTo[1];
    mx = (d->m_shape[1].x() - x) / strokeLength;
    my = (d->m_shape[1].y() - y) / strokeLength;
    position = 0.0;

    //set up lower bound of search interval on other shape
    ox = o_shape[0].x();
    oy = o_shape[0].y();
    strokeLength = o_lengthTo[1];
    omx = (o_shape[1].x() - ox) / strokeLength;
    omy = (o_shape[1].y() - oy) / strokeLength;
    opposition = 0.0;
    dist = metric(ox - x, oy - y);

    for (int i = 0; i <= 30; i++) {
        //go to comparison point on our own polygon
        //30.0001 to prevent getting out-of-bounds pointIndex
        desiredPosition = d->m_curveLength / 30.0001 * (float)i;
        if (desiredPosition > d->m_lengthTo[pointIndex + 1]) {

            while (desiredPosition > d->m_lengthTo[pointIndex + 1]) {
                pointIndex++;
            }

            x = d->m_shape[pointIndex].x();
            y = d->m_shape[pointIndex].y();
            position = d->m_lengthTo[pointIndex];
            strokeLength = d->m_lengthTo[pointIndex + 1] - position;
            mx = (d->m_shape[pointIndex + 1].x() - x) / strokeLength;
            my = (d->m_shape[pointIndex + 1].y() - y) / strokeLength;
        }
        x += mx * (desiredPosition - position);
        y += my * (desiredPosition - position);
        position = desiredPosition;

        //set up upper bound of search interval on other shape
        desiredPosition = qMin(opposition + other.d->m_curveLength / 15.00005,
                               other.d->m_curveLength - 0.0001);
        if (i == 0 || desiredPosition > o_lengthTo[opointIndexB + 1]) {

            while (desiredPosition > o_lengthTo[opointIndexB + 1]) {
                opointIndexB++;
            }

            oxB = o_shape[opointIndexB].x();
            oyB = o_shape[opointIndexB].y();
            oppositionB = o_lengthTo[opointIndexB];
            strokeLength = o_lengthTo[opointIndexB + 1] - oppositionB;
            omxB = (o_shape[opointIndexB + 1].x() - oxB) / strokeLength;
            omyB = (o_shape[opointIndexB + 1].y() - oyB) / strokeLength;
        }
        oxB += omxB * (desiredPosition - oppositionB);
        oyB += omyB * (desiredPosition - oppositionB);
        oppositionB = desiredPosition;
        distB = metric(oxB - x, oyB - y);

        //binary search for nearest point on other shape
        for (int j = 0; j < 6; j++) {
            desiredPosition = (opposition + oppositionB) * 0.5;
            if (dist < distB) {
                //retract upper bound to desiredPosition
                //copy state of lower bound to upper bound, advance it from there
                oxB = ox; oyB = oy;
                omxB = omx; omyB = omy;
                opointIndexB = opointIndex; oppositionB = opposition;

                if (desiredPosition > o_lengthTo[opointIndexB + 1]) {

                    while (desiredPosition > o_lengthTo[opointIndexB + 1]) {
                        opointIndexB++;
                    }

                    oxB = o_shape[opointIndexB].x();
                    oyB = o_shape[opointIndexB].y();
                    oppositionB = o_lengthTo[opointIndexB];
                    strokeLength = o_lengthTo[opointIndexB + 1] - oppositionB;
                    omxB = (o_shape[opointIndexB + 1].x() - oxB) / strokeLength;
                    omyB = (o_shape[opointIndexB + 1].y() - oyB) / strokeLength;
                }
                oxB += omxB * (desiredPosition - oppositionB);
                oyB += omyB * (desiredPosition - oppositionB);
                oppositionB = desiredPosition;
                distB = metric(oxB - x, oyB - y);
            } else {
                //advance lower bound to desiredPosition
                if (desiredPosition > o_lengthTo[opointIndex + 1]) {

                    while (desiredPosition > o_lengthTo[opointIndex + 1]) {
                        opointIndex++;
                    }

                    ox = o_shape[opointIndex].x();
                    oy = o_shape[opointIndex].y();
                    opposition = o_lengthTo[opointIndex];
                    strokeLength = o_lengthTo[opointIndex + 1] - opposition;
                    omx = (o_shape[opointIndex + 1].x() - ox) / strokeLength;
                    omy = (o_shape[opointIndex + 1].y() - oy) / strokeLength;
                }
                ox += omx * (desiredPosition - opposition);
                oy += omy * (desiredPosition - opposition);
                opposition = desiredPosition;
                dist = metric(ox - x, oy - y);
            }
        }
        retval += qMin(dist, distB);
    }
    //scale value to make it roughly invariant against step width
    return retval / 30.0;
}

KisKShapeGesture &KisKShapeGesture::operator=(const KisKShapeGesture &other)
{
    d->m_lengthTo = other.d->m_lengthTo;
    d->m_shape = other.d->m_shape;
    d->m_curveLength = other.d->m_curveLength;
    return *this;
}

bool KisKShapeGesture::operator==(const KisKShapeGesture &other) const
{
    //a really fast and workable shortcut
    if (fabs(d->m_curveLength - other.d->m_curveLength) > 0.1) {
        return false;
    }
    return d->m_shape == other.d->m_shape;
}

bool KisKShapeGesture::operator!=(const KisKShapeGesture &other) const
{
    return !operator==(other);
}

uint KisKShapeGesture::hashable() const
{
    uint hash = 0;

    foreach (const QPoint &point, d->m_shape) {
        hash += qHash(point.x()) + qHash(point.y());
    }

    return hash;
}

/********************************************************
 * KisKRockerGesture *
 *******************************************************/

class KisKRockerGesturePrivate
{
public:
    KisKRockerGesturePrivate()
        : m_hold(Qt::NoButton),
          m_thenPush(Qt::NoButton)
    {
    }
    KisKRockerGesturePrivate(const KisKRockerGesturePrivate &other)
        : m_hold(other.m_hold),
          m_thenPush(other.m_thenPush)
    {
    }
    Qt::MouseButton m_hold;
    Qt::MouseButton m_thenPush;
};

KisKRockerGesture::KisKRockerGesture()
    : d(new KisKRockerGesturePrivate)
{
}

KisKRockerGesture::KisKRockerGesture(Qt::MouseButton hold, Qt::MouseButton thenPush)
    : d(new KisKRockerGesturePrivate)
{
    setButtons(hold, thenPush);
}

KisKRockerGesture::KisKRockerGesture(const QString &description)
    : d(new KisKRockerGesturePrivate)
{
    if (description.length() != 2) {
        return;
    }

    Qt::MouseButton hold, thenPush;
    Qt::MouseButton *current = &hold;
    for (int i = 0; i < 2; i++) {
        switch (description[i].toLatin1()) {
        case 'L':
            *current = Qt::LeftButton;
            break;
        case 'R':
            *current = Qt::RightButton;
            break;
        case 'M':
            *current = Qt::MiddleButton;
            break;
        case '1':
            *current = Qt::XButton1;
            break;
        case '2':
            *current = Qt::XButton2;
            break;
        default:
            return;
        }
        current = &thenPush;
    }
    d->m_hold = hold;
    d->m_thenPush = thenPush;
}

KisKRockerGesture::KisKRockerGesture(const KisKRockerGesture &other)
    : d(new KisKRockerGesturePrivate(*(other.d)))
{
}

KisKRockerGesture::~KisKRockerGesture()
{
    delete d;
}

void KisKRockerGesture::setButtons(Qt::MouseButton hold, Qt::MouseButton thenPush)
{
    if (hold == thenPush) {
        d->m_hold = Qt::NoButton;
        d->m_thenPush = Qt::NoButton;
        return;
    }

    int button = hold;
    for (int i = 0; i < 2; i++) {
        switch (button) {
        case Qt::LeftButton:
        case Qt::RightButton:
        case Qt::MiddleButton:
        case Qt::XButton1:
        case Qt::XButton2:
            break;
        default:
            d->m_hold = Qt::NoButton;
            d->m_thenPush = Qt::NoButton;
            return;
        }
        button = thenPush;
    }

    d->m_hold = hold;
    d->m_thenPush = thenPush;
}

void KisKRockerGesture::getButtons(Qt::MouseButton *hold, Qt::MouseButton *thenPush) const
{
    *hold = d->m_hold;
    *thenPush = d->m_thenPush;
}

QString KisKRockerGesture::mouseButtonName(Qt::MouseButton button)
{
    switch (button) {
    case Qt::LeftButton:
        return i18nc("left mouse button", "left button");
        break;
    case Qt::MiddleButton:
        return i18nc("middle mouse button", "middle button");
        break;
    case Qt::RightButton:
        return i18nc("right mouse button", "right button");
        break;
    default:
        return i18nc("a nonexistent value of mouse button", "invalid button");
        break;
    }
}

QString KisKRockerGesture::rockerName() const
{
    if (!isValid()) {
        return QString();
    }
    //return i18nc("an invalid mouse gesture of type \"hold down one button, then press another button\"",
    //             "invalid rocker gesture");
    else
        return i18nc("a kind of mouse gesture: hold down one mouse button, then press another button",
                     "Hold %1, then push %2", mouseButtonName(d->m_hold), mouseButtonName(d->m_thenPush));
}

bool KisKRockerGesture::isValid() const
{
    return (d->m_hold != Qt::NoButton);
}

QString KisKRockerGesture::toString() const
{
    if (!isValid()) {
        return QString();
    }
    QString ret;
    int button = d->m_hold;
    char desc;
    for (int i = 0; i < 2; i++) {
        switch (button) {
        case Qt::LeftButton:
            desc = 'L';
            break;
        case Qt::RightButton:
            desc = 'R';
            break;
        case Qt::MiddleButton:
            desc = 'M';
            break;
        case Qt::XButton1:
            desc = '1';
            break;
        case Qt::XButton2:
            desc = '2';
            break;
        default:
            return QString();
        }
        ret.append(QLatin1Char(desc));
        button = d->m_thenPush;
    }
    return ret;
}

KisKRockerGesture &KisKRockerGesture::operator=(const KisKRockerGesture &other)
{
    d->m_hold = other.d->m_hold;
    d->m_thenPush = other.d->m_thenPush;
    return *this;
}

bool KisKRockerGesture::operator==(const KisKRockerGesture &other) const
{
    return d->m_hold == other.d->m_hold && d->m_thenPush == other.d->m_thenPush;
}

bool KisKRockerGesture::operator!=(const KisKRockerGesture &other) const
{
    return !operator==(other);
}

uint KisKRockerGesture::hashable() const
{
    //make it asymmetric
    return qHash(d->m_hold) + d->m_thenPush;
}
