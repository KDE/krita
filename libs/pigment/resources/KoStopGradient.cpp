/*
    Copyright (C) 2005 Tim Beaulen <tbscope@gmail.org>
    Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KoStopGradient.h"

#include <cfloat>

#include <QColor>
#include <QFile>
#include <QDomDocument>

#include <klocale.h>
#include <kdebug.h>

#include "KoColorSpaceRegistry.h"

#include <math.h>
#include <KoColorModelStandardIds.h>

KoStopGradient::KoStopGradient(const QString& filename)
        : KoAbstractGradient(filename)
{
}

KoStopGradient::~KoStopGradient()
{
}

bool KoStopGradient::load()
{
    QString strExt;
    const int result = filename().lastIndexOf('.');
    if (result >= 0) {
        strExt = filename().mid(result).toLower();
    }
    QFile f(filename());

    if (f.open(QIODevice::ReadOnly)) {
        if (strExt == ".kgr") {
            loadKarbonGradient(&f);
        } else if (strExt == ".svg") {
            loadSvgGradient(&f);
        }
    }
    if (m_stops.count() >= 2)
        setValid(true);

    updatePreview();

    return true;
}

bool KoStopGradient::save()
{
    QFile fileOut(filename());
    if (! fileOut.open(QIODevice::WriteOnly))
        return false;

    QTextStream stream(&fileOut);

    const QString spreadMethod[3] = {
        QString("spreadMethod=\"pad\" "),
        QString("spreadMethod=\"reflect\" "),
        QString("spreadMethod=\"repeat\" ")
    };

    const QString indent = "    ";

    stream << "<svg>" << endl;

    stream << indent;
    stream << "<linearGradient id=\"" << name() << "\" ";
    stream << "gradientUnits=\"objectBoundingBox\" ";
    stream << spreadMethod[spread()];
    stream << ">" << endl;

    QColor color;

    // color stops
    foreach(const KoGradientStop & stop, m_stops) {
        stop.second.toQColor(&color);
        stream << indent << indent;
        stream << "<stop stop-color=\"";
        stream << color.name();
        stream << "\" offset=\"" << QString().setNum(stop.first);
        stream << "\" stop-opacity=\"" << static_cast<float>(color.alpha()) / 255.0f << "\"" << " />" << endl;
    }
    stream << indent;
    stream << "</linearGradient>" << endl;
    stream << "</svg>" << endl;

    fileOut.close();

    return true;
}

QGradient* KoStopGradient::toQGradient() const
{
    QGradient* gradient;

    switch (type()) {
    case QGradient::LinearGradient: {
        gradient = new QLinearGradient(m_start, m_stop);
        break;
    }
    case QGradient::RadialGradient: {
        QPointF diff = m_stop - m_start;
        qreal radius = sqrt(diff.x() * diff.x() + diff.y() * diff.y());
        gradient = new QRadialGradient(m_start, radius, m_focalPoint);
        break;
    }
    case QGradient::ConicalGradient: {
        QPointF diff = m_stop - m_start;
        qreal angle = atan2(m_start.y(), m_start.x()) * 180.0 / M_PI;
        if (angle < 0.0)
            angle += 360.0;
        gradient = new QConicalGradient(m_start, angle);
        break;
    }
    default:
        return 0;
    }
    QColor color;
    for (QList<KoGradientStop>::const_iterator i = m_stops.begin(); i != m_stops.end(); ++i) {
        i->second.toQColor(&color);
        gradient->setColorAt(i->first , color);
    }
    return gradient;
}

void KoStopGradient::colorAt(KoColor& dst, qreal t) const
{
    if (! m_stops.count())
        return;
    if (t <= m_stops.first().first || m_stops.count() == 1) {
        // we have only one stop or t is before the first stop
        // -> use the color of the first stop
        dst.fromKoColor(m_stops.first().second);
    } else if (t >= m_stops.last().first) {
        // t is after the last stop
        // -> use the color of the last stop
        dst.fromKoColor(m_stops.last().second);
    } else {
        // we have at least two color stops
        // -> find the two stops which frame our t
        QList<KoGradientStop>::const_iterator stop = m_stops.begin();
        QList<KoGradientStop>::const_iterator lastStop = m_stops.end();
        // we already checked the first stop, so we start at the second
        for (++stop; stop != lastStop; ++stop) {
            // we break at the stop which is just after our t
            if (stop->first > t)
                break;
        }

        if ( !(*buffer.colorSpace() == *colorSpace())) {
            buffer = KoColor(colorSpace());
        }

        const KoGradientStop& leftStop = *(stop - 1);
        const KoGradientStop& rightStop = *(stop);

        const quint8 *colors[2];
        colors[0] = leftStop.second.data();
        colors[1] = rightStop.second.data();

        qreal localT;
        qreal stopDistance = rightStop.first - leftStop.first;
        if (stopDistance < DBL_EPSILON) {
            localT = 0.5;
        } else {
            localT = (t - leftStop.first) / stopDistance;
        }
        qint16 colorWeights[2];
        colorWeights[0] = static_cast<quint8>((1.0 - localT) * 255 + 0.5);
        colorWeights[1] = 255 - colorWeights[0];

        colorSpace()->mixColorsOp()->mixColors(colors, colorWeights, 2, buffer.data());

        dst.fromKoColor(buffer);
    }
}

KoStopGradient * KoStopGradient::fromQGradient(QGradient * gradient)
{
    if (! gradient)
        return 0;

    KoStopGradient * newGradient = new KoStopGradient("");
    newGradient->setType(gradient->type());
    newGradient->setSpread(gradient->spread());

    switch (gradient->type()) {
    case QGradient::LinearGradient: {
        QLinearGradient * g = static_cast<QLinearGradient*>(gradient);
        newGradient->m_start = g->start();
        newGradient->m_stop = g->finalStop();
        newGradient->m_focalPoint = g->start();
        break;
    }
    case QGradient::RadialGradient: {
        QRadialGradient * g = static_cast<QRadialGradient*>(gradient);
        newGradient->m_start = g->center();
        newGradient->m_stop = g->center() + QPointF(g->radius(), 0);
        newGradient->m_focalPoint = g->focalPoint();
        break;
    }
    case QGradient::ConicalGradient: {
        QConicalGradient * g = static_cast<QConicalGradient*>(gradient);
        qreal radian = g->angle() * M_PI / 180.0;
        newGradient->m_start = g->center();
        newGradient->m_stop = QPointF(100.0 * cos(radian), 100.0 * sin(radian));
        newGradient->m_focalPoint = g->center();
        break;
    }
    default:
        delete newGradient;
        return 0;
    }

    foreach(const QGradientStop & stop, gradient->stops()) {
        KoColor color;
        color.fromQColor(stop.second);
        newGradient->m_stops.append(KoGradientStop(stop.first, color));
    }

    return newGradient;
}

void KoStopGradient::setStops(QList< KoGradientStop > stops)
{
    m_stops.clear();
    KoColor color;
    foreach(const KoGradientStop & stop, stops) {
        color = stop.second;
        color.convertTo(colorSpace());
        m_stops.append(KoGradientStop(stop.first, color));
    }
    updatePreview();
}

void KoStopGradient::loadKarbonGradient(QFile* file)
{
    QDomDocument doc;

    if (!(doc.setContent(file))) {
        file->close();
        setValid(false);
        return;
    }

    QDomElement e;
    QDomNode n = doc.documentElement().firstChild();

    if (!n.isNull()) {
        e = n.toElement();
        if (!e.isNull() && e.tagName() == "GRADIENT")
            parseKarbonGradient(e);
    }
}

void KoStopGradient::loadSvgGradient(QFile* file)
{
    QDomDocument doc;

    if (!(doc.setContent(file)))
        file->close();
    else {
        for (QDomNode n = doc.documentElement().firstChild(); !n.isNull(); n = n.nextSibling()) {
            QDomElement e = n.toElement();

            if (e.isNull()) continue;

            if (e.tagName() == "linearGradient" || e.tagName() == "radialGradient") {
                parseSvgGradient(e);
                return;
            }
            // Inkscape gradients are in another defs
            if (e.tagName() == "defs") {
                for (QDomNode defnode = e.firstChild(); !defnode.isNull(); defnode = defnode.nextSibling()) {
                    QDomElement defelement = defnode.toElement();

                    if (defelement.isNull()) continue;

                    if (defelement.tagName() == "linearGradient" || defelement.tagName() == "radialGradient") {
                        parseSvgGradient(defelement);
                        return;
                    }
                }
            }
        }
    }
}

void KoStopGradient::parseKarbonGradient(const QDomElement& element)
{
    m_start = QPointF(element.attribute("originX", "0.0").toDouble(), element.attribute("originY", "0.0").toDouble());
    m_focalPoint = QPointF(element.attribute("focalX", "0.0").toDouble(), element.attribute("focalY", "0.0").toDouble());
    m_stop = QPointF(element.attribute("vectorX", "0.0").toDouble(), element.attribute("vectorY", "0.0").toDouble());

    setType((QGradient::Type)element.attribute("type", 0).toInt());
    setSpread((QGradient::Spread)element.attribute("repeatMethod", 0).toInt());

    m_stops.clear();

    qreal color1, color2, color3, color4, opacity;
    KoColor color;
    // load stops
    QDomNodeList list = element.childNodes();
    for (int i = 0; i < list.count(); ++i) {

        if (list.item(i).isElement()) {
            QDomElement colorstop = list.item(i).toElement();

            if (colorstop.tagName() == "COLORSTOP") {
                QDomElement e = colorstop.firstChild().toElement();

                opacity = e.attribute("opacity", "1.0").toFloat();

                QColor tmpColor;
                const KoColorSpace* stopColorSpace;
                switch (e.attribute("colorSpace").toUShort()) {
                case 1:  // cmyk
                    color1 = e.attribute("v1", "0.0").toFloat();
                    color2 = e.attribute("v2", "0.0").toFloat();
                    color3 = e.attribute("v3", "0.0").toFloat();
                    color4 = e.attribute("v4", "0.0").toFloat();

                    stopColorSpace = KoColorSpaceRegistry::instance()->colorSpace( CMYKAColorModelID.id(), Integer8BitsColorDepthID.id(), QString());
                    if (stopColorSpace) {
                        quint8 data[5];
                        data[0] = static_cast<quint8>(color1 * 255 + 0.5);
                        data[1] = static_cast<quint8>(color2 * 255 + 0.5);
                        data[2] = static_cast<quint8>(color3 * 255 + 0.5);
                        data[3] = static_cast<quint8>(color4 * 255 + 0.5);
                        data[4] = static_cast<quint8>(opacity * OPACITY_OPAQUE_U8 + 0.5);
                        color.setColor(data, stopColorSpace);
                    } else {
                        // cmyk colorspace not found fallback to rgb
                        color.convertTo(KoColorSpaceRegistry::instance()->rgb8());
                        tmpColor.setCmykF(color1, color2, color3, color4);
                        tmpColor.setAlpha(static_cast<quint8>(opacity * OPACITY_OPAQUE_U8 + 0.5));
                        color.fromQColor(tmpColor);
                    }
                    break;
                case 2: // hsv
                    color1 = e.attribute("v1", "0.0").toFloat();
                    color2 = e.attribute("v2", "0.0").toFloat();
                    color3 = e.attribute("v3", "0.0").toFloat();

                    color.convertTo(KoColorSpaceRegistry::instance()->rgb8());
                    tmpColor.setHsvF(color1, color2, color3);
                    tmpColor.setAlpha(static_cast<quint8>(opacity * OPACITY_OPAQUE_U8 + 0.5));
                    color.fromQColor(tmpColor);
                    break;
                case 3: // gray
                    color1 = e.attribute("v1", "0.0").toFloat();
                    stopColorSpace = KoColorSpaceRegistry::instance()->colorSpace( GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), QString());
                    if (stopColorSpace) {
                        quint8 data[2];
                        data[0] = static_cast<quint8>(color1 * 255 + 0.5);
                        data[1] = static_cast<quint8>(opacity * OPACITY_OPAQUE_U8 + 0.5);
                        color.setColor(data, stopColorSpace);
                    } else {
                        // gray colorspace not found fallback to rgb
                        color.convertTo(KoColorSpaceRegistry::instance()->rgb8());
                        tmpColor.setRgbF(color1, color1, color1);
                        tmpColor.setAlpha(static_cast<quint8>(opacity * OPACITY_OPAQUE_U8 + 0.5));
                        color.fromQColor(tmpColor);
                    }
                    break;
                default: // rgb
                    color1 = e.attribute("v1", "0.0").toFloat();
                    color2 = e.attribute("v2", "0.0").toFloat();
                    color3 = e.attribute("v3", "0.0").toFloat();
                    stopColorSpace = KoColorSpaceRegistry::instance()->rgb8();

                    quint8 data[4];
                    data[2] = static_cast<quint8>(color1 * 255 + 0.5);
                    data[1] = static_cast<quint8>(color2 * 255 + 0.5);
                    data[0] = static_cast<quint8>(color3 * 255 + 0.5);
                    data[3] = static_cast<quint8>(opacity * OPACITY_OPAQUE_U8 + 0.5);

                    color.setColor(data, stopColorSpace);
                }

                qreal offset = colorstop.attribute("ramppoint", "0.0").toFloat();
//              midpoint = colorstop.attribute("midpoint", "0.5").toFloat();

                m_stops.append(KoGradientStop(offset, color));
            }
        }
    }
}

void KoStopGradient::parseSvgGradient(const QDomElement& element)
{
    m_stops.clear();
    setSpread(QGradient::PadSpread);

    /*QString href = e.attribute( "xlink:href" ).mid( 1 );
    if( !href.isEmpty() )
    {
    }*/
    setName(element.attribute("id", i18n("SVG Gradient")));

    const KoColorSpace* rgbColorSpace = KoColorSpaceRegistry::instance()->rgb8();

    bool bbox = element.attribute("gradientUnits") != "userSpaceOnUse";

    if (element.tagName() == "linearGradient") {

        if (bbox) {
            QString s;

            s = element.attribute("x1", "0%");
            qreal xOrigin;
            if (s.endsWith('%'))
                xOrigin = s.remove('%').toDouble();
            else
                xOrigin = s.toDouble() * 100.0;

            s = element.attribute("y1", "0%");
            qreal yOrigin;
            if (s.endsWith('%'))
                yOrigin = s.remove('%').toDouble();
            else
                yOrigin = s.toDouble() * 100.0;

            s = element.attribute("x2", "100%");
            qreal xVector;
            if (s.endsWith('%'))
                xVector = s.remove('%').toDouble();
            else
                xVector = s.toDouble() * 100.0;

            s = element.attribute("y2", "0%");
            qreal yVector;
            if (s.endsWith('%'))
                yVector = s.remove('%').toDouble();
            else
                yVector = s.toDouble() * 100.0;

            m_start = QPointF(xOrigin, yOrigin);
            m_stop = QPointF(xVector, yVector);
        } else {
            m_start = QPointF(element.attribute("x1").toDouble(), element.attribute("y1").toDouble());
            m_stop = QPointF(element.attribute("x2").toDouble(), element.attribute("y2").toDouble());
        }
        setType(QGradient::LinearGradient);
    } else {
        if (bbox) {
            QString s;

            s = element.attribute("cx", "50%");
            qreal xOrigin;
            if (s.endsWith('%'))
                xOrigin = s.remove('%').toDouble();
            else
                xOrigin = s.toDouble() * 100.0;

            s = element.attribute("cy", "50%");
            qreal yOrigin;
            if (s.endsWith('%'))
                yOrigin = s.remove('%').toDouble();
            else
                yOrigin = s.toDouble() * 100.0;

            s = element.attribute("cx", "50%");
            qreal xVector;
            if (s.endsWith('%'))
                xVector = s.remove('%').toDouble();
            else
                xVector = s.toDouble() * 100.0;

            s = element.attribute("r", "50%");
            if (s.endsWith('%'))
                xVector += s.remove('%').toDouble();
            else
                xVector += s.toDouble() * 100.0;

            s = element.attribute("cy", "50%");
            qreal yVector;
            if (s.endsWith('%'))
                yVector = s.remove('%').toDouble();
            else
                yVector = s.toDouble() * 100.0;

            s = element.attribute("fx", "50%");
            qreal xFocal;
            if (s.endsWith('%'))
                xFocal = s.remove('%').toDouble();
            else
                xFocal = s.toDouble() * 100.0;

            s = element.attribute("fy", "50%");
            qreal yFocal;
            if (s.endsWith('%'))
                yFocal = s.remove('%').toDouble();
            else
                yFocal = s.toDouble() * 100.0;

            m_start = QPointF(xOrigin, yOrigin);
            m_stop = QPointF(xVector, yVector);
            m_focalPoint = QPointF(xFocal, yFocal);
        } else {
            m_start = QPointF(element.attribute("cx").toDouble(), element.attribute("cy").toDouble());
            m_stop = QPointF(element.attribute("cx").toDouble() + element.attribute("r").toDouble(),
                             element.attribute("cy").toDouble());
            m_focalPoint = QPointF(element.attribute("fx").toDouble(), element.attribute("fy").toDouble());
        }
        setType(QGradient::RadialGradient);
    }
    // handle spread method
    QString spreadMethod = element.attribute("spreadMethod");
    if (!spreadMethod.isEmpty()) {
        if (spreadMethod == "reflect")
            setSpread(QGradient::ReflectSpread);
        else if (spreadMethod == "repeat")
            setSpread(QGradient::RepeatSpread);
    }

    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement colorstop = n.toElement();
        if (colorstop.tagName() == "stop") {
            qreal opacity = 0.0;
            QColor c;
            float off;
            QString temp = colorstop.attribute("offset");
            if (temp.contains('%')) {
                temp = temp.left(temp.length() - 1);
                off = temp.toFloat() / 100.0;
            } else
                off = temp.toFloat();

            if (!colorstop.attribute("stop-color").isEmpty())
                parseSvgColor(c, colorstop.attribute("stop-color"));
            else {
                // try style attr
                QString style = colorstop.attribute("style").simplified();
                QStringList substyles = style.split(';', QString::SkipEmptyParts);
                foreach(const QString & s, substyles) {
                    QStringList substyle = s.split(':');
                    QString command = substyle[0].trimmed();
                    QString params = substyle[1].trimmed();
                    if (command == "stop-color")
                        parseSvgColor(c, params);
                    if (command == "stop-opacity")
                        opacity = params.toDouble();
                }

            }
            if (!colorstop.attribute("stop-opacity").isEmpty())
                opacity = colorstop.attribute("stop-opacity").toDouble();

            KoColor color(rgbColorSpace);
            color.fromQColor(c);
            color.setOpacity(static_cast<quint8>(opacity * OPACITY_OPAQUE_U8 + 0.5));

            //According to the SVG spec each gradient offset has to be equal to or greater than the previous one
            //if not it needs to be adjusted to be equal
            if (m_stops.count() > 0 && m_stops.last().first >= off) {
                off = m_stops.last().first;
            }
            m_stops.append(KoGradientStop(off, color));
        }
    }
}

void KoStopGradient::parseSvgColor(QColor &color, const QString &s)
{
    if (s.startsWith("rgb(")) {
        QString parse = s.trimmed();
        QStringList colors = parse.split(',');
        QString r = colors[0].right((colors[0].length() - 4));
        QString g = colors[1];
        QString b = colors[2].left((colors[2].length() - 1));

        if (r.contains("%")) {
            r = r.left(r.length() - 1);
            r = QString::number(int((qreal(255 * r.toDouble()) / 100.0)));
        }

        if (g.contains("%")) {
            g = g.left(g.length() - 1);
            g = QString::number(int((qreal(255 * g.toDouble()) / 100.0)));
        }

        if (b.contains("%")) {
            b = b.left(b.length() - 1);
            b = QString::number(int((qreal(255 * b.toDouble()) / 100.0)));
        }

        color = QColor(r.toInt(), g.toInt(), b.toInt());
    } else {
        QString rgbColor = s.trimmed();
        QColor c;
        if (rgbColor.startsWith('#'))
            c.setNamedColor(rgbColor);
        else {
            c = QColor(rgbColor);
        }
        color = c;
    }
}

QString KoStopGradient::defaultFileExtension() const
{
    return QString(".svg");
}
