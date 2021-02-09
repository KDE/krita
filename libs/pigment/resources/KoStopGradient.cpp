/*
    SPDX-FileCopyrightText: 2005 Tim Beaulen <tbscope@gmail.org>
    SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
    SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <resources/KoStopGradient.h>

#include <cfloat>

#include <QColor>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QBuffer>

#include <klocalizedstring.h>
#include <DebugPigment.h>

#include "KoColorSpaceRegistry.h"
#include <KoColorSpaceEngine.h>
#include <KoColorProfile.h>
#include "KoMixColorsOp.h"

#include "kis_dom_utils.h"

#include <math.h>
#include <KoColorModelStandardIds.h>
#include <KoXmlNS.h>

#include <KoCanvasResourcesIds.h>
#include <KoCanvasResourcesInterface.h>


KoStopGradient::KoStopGradient(const QString& filename)
    : KoAbstractGradient(filename)
{
}

KoStopGradient::~KoStopGradient()
{
}

KoStopGradient::KoStopGradient(const KoStopGradient &rhs)
    : KoAbstractGradient(rhs),
      m_stops(rhs.m_stops),
      m_start(rhs.m_start),
      m_stop(rhs.m_stop),
      m_focalPoint(rhs.m_focalPoint)
{
}

bool KoStopGradient::operator==(const KoStopGradient& rhs) const
{
    return
        *colorSpace() == *rhs.colorSpace() &&
        spread() == rhs.spread() &&
        type() == rhs.type() &&
        m_start == rhs.m_start &&
        m_stop == rhs.m_stop &&
        m_focalPoint == rhs.m_focalPoint &&
            m_stops == rhs.m_stops;
}

KoResourceSP KoStopGradient::clone() const
{
    return KoResourceSP(new KoStopGradient(*this));
}

bool KoStopGradient::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

    QString strExt;
    const int result = filename().lastIndexOf('.');
    if (result >= 0) {
        strExt = filename().mid(result).toLower();
    }
    QByteArray ba = dev->readAll();

    QBuffer buf(&ba);
    loadSvgGradient(&buf);
    if (m_stops.count() >= 2) {
        setValid(true);
    }
    updatePreview();
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
        i->color.toQColor(&color);
        gradient->setColorAt(i->position, color);
    }

    gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient->setSpread(this->spread());

    return gradient;
}

bool KoStopGradient::stopsAt(KoGradientStop& leftStop, KoGradientStop& rightStop, qreal t) const
{
    if (!m_stops.count())
        return false;
    if (t <= m_stops.first().position || m_stops.count() == 1) {
        // we have only one stop or t is before the first stop
        leftStop = m_stops.first();
        rightStop = KoGradientStop(-std::numeric_limits<double>::infinity(), leftStop.color, leftStop.type);
        return true;
    } else if (t >= m_stops.last().position) {
        // t is after the last stop
        rightStop = m_stops.last();
        leftStop = KoGradientStop(std::numeric_limits<double>::infinity(), rightStop.color, rightStop.type);
        return true;
    } else {
        // we have at least two color stops
        // -> find the two stops which frame our t
        auto it = std::lower_bound(m_stops.begin(), m_stops.end(), KoGradientStop(t, KoColor(), COLORSTOP), [](const KoGradientStop& a, const KoGradientStop& b) {
            return a.position < b.position;
            });
        leftStop = *(it - 1);
        rightStop = *(it);
        return true;
    }
}

void KoStopGradient::colorAt(KoColor& dst, qreal t) const
{
    KoColor buffer;

    KoGradientStop leftStop, rightStop;
    if (!stopsAt(leftStop, rightStop, t)) return;

    const KoColorSpace* mixSpace = KoColorSpaceRegistry::instance()->rgb8(dst.colorSpace()->profile());

    KoColor startDummy, endDummy;
    if (mixSpace) {
        startDummy = KoColor(leftStop.color, mixSpace);
        endDummy = KoColor(rightStop.color, mixSpace);
    } else {
        startDummy = leftStop.color;
        endDummy = rightStop.color;
    }
    const quint8* colors[2];
    colors[0] = startDummy.data();
    colors[1] = endDummy.data();

    qreal localT;
    qreal stopDistance = rightStop.position - leftStop.position;
    if (stopDistance < DBL_EPSILON) {
        localT = 0.5;
    } else {
        localT = (t - leftStop.position) / stopDistance;
    }
    qint16 colorWeights[2];
    colorWeights[0] = static_cast<quint8>((1.0 - localT) * 255 + 0.5);
    colorWeights[1] = 255 - colorWeights[0];

    //check if our mixspace exists, it doesn't at startup.
    if (mixSpace) {
        if (*buffer.colorSpace() != *mixSpace) {
            buffer = KoColor(mixSpace);
        }
        mixSpace->mixColorsOp()->mixColors(colors, colorWeights, 2, buffer.data());
    } else {
        buffer = KoColor(colorSpace());
        colorSpace()->mixColorsOp()->mixColors(colors, colorWeights, 2, buffer.data());
    }

    dst.fromKoColor(buffer);
}

QSharedPointer<KoStopGradient> KoStopGradient::fromQGradient(const QGradient *gradient)
{
    if (!gradient)
        return QSharedPointer<KoStopGradient>(0);

    QSharedPointer<KoStopGradient> newGradient(new KoStopGradient(QString()));
    newGradient->setType(gradient->type());
    newGradient->setSpread(gradient->spread());

    switch (gradient->type()) {
    case QGradient::LinearGradient: {
        const QLinearGradient* g = static_cast<const QLinearGradient*>(gradient);
        newGradient->m_start = g->start();
        newGradient->m_stop = g->finalStop();
        newGradient->m_focalPoint = g->start();
        break;
    }
    case QGradient::RadialGradient: {
        const QRadialGradient* g = static_cast<const QRadialGradient*>(gradient);
        newGradient->m_start = g->center();
        newGradient->m_stop = g->center() + QPointF(g->radius(), 0);
        newGradient->m_focalPoint = g->focalPoint();
        break;
    }
    case QGradient::ConicalGradient: {
        const QConicalGradient* g = static_cast<const QConicalGradient*>(gradient);
        qreal radian = g->angle() * M_PI / 180.0;
        newGradient->m_start = g->center();
        newGradient->m_stop = QPointF(100.0 * cos(radian), 100.0 * sin(radian));
        newGradient->m_focalPoint = g->center();
        break;
    }
    default:
        return QSharedPointer<KoStopGradient>(0);;
    }

    Q_FOREACH(const QGradientStop & stop, gradient->stops()) {
        KoColor color(newGradient->colorSpace());
        color.fromQColor(stop.second);
        newGradient->m_stops.append(KoGradientStop(stop.first, color, COLORSTOP));
    }

    newGradient->setValid(true);

    return newGradient;
}

void KoStopGradient::setStops(QList< KoGradientStop > stops)
{
    m_stops.clear();
    m_hasVariableStops = false;
    KoColor color;
    Q_FOREACH(const KoGradientStop & stop, stops) {
        color = stop.color;
        color.convertTo(colorSpace());
        m_stops.append(KoGradientStop(stop.position, color, stop.type));
        if (stop.type != COLORSTOP) {
            m_hasVariableStops = true;
        }
    }
    updatePreview();
}

QList<KoGradientStop> KoStopGradient::stops() const
{
    return m_stops;
}

QList<int> KoStopGradient::requiredCanvasResources() const
{
    QList<int> result;

    if (std::find_if(m_stops.begin(), m_stops.end(),
                     [] (const KoGradientStop &stop) {
                         return stop.type != COLORSTOP;
                     }) != m_stops.end()) {

        result << KoCanvasResource::ForegroundColor << KoCanvasResource::BackgroundColor;
    }

    return result;
}

void KoStopGradient::bakeVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    const KoColor fgColor = canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    const KoColor bgColor = canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>();

    for (auto it = m_stops.begin(); it != m_stops.end(); ++it) {
        if (it->type == FOREGROUNDSTOP) {
            it->color = fgColor;
            it->type = COLORSTOP;
        } else if (it->type == BACKGROUNDSTOP) {
            it->color = bgColor;
            it->type = COLORSTOP;
        }
    }
}

void KoStopGradient::updateVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    const KoColor fgColor = canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    const KoColor bgColor = canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>();

    for (auto it = m_stops.begin(); it != m_stops.end(); ++it) {
        if (it->type == FOREGROUNDSTOP) {
            it->color = fgColor;
        } else if (it->type == BACKGROUNDSTOP) {
            it->color = bgColor;
        }
    }
}

void KoStopGradient::loadSvgGradient(QIODevice* file)
{
    QDomDocument doc;

    if (!(doc.setContent(file))) {
        file->close();
    } else {
        QHash<QString, const KoColorProfile*> profiles;
        for (QDomElement e = doc.documentElement().firstChildElement("defs"); !e.isNull(); e = e.nextSiblingElement("defs")) {
            for (QDomElement profileEl = e.firstChildElement("color-profile"); !profileEl.isNull(); profileEl = profileEl.nextSiblingElement("color-profile")) {
                const QString href = profileEl.attribute("xlink:href");
                const QByteArray uniqueId = QByteArray::fromHex(profileEl.attribute("local").toLatin1());
                const QString name = profileEl.attribute("name");

                const KoColorProfile *profile =
                        KoColorSpaceRegistry::instance()->profileByUniqueId(uniqueId);
                if (!profile) {
                    QFile file(href);
                    if (file.exists()) {
                        KoColorSpaceEngine *engine = KoColorSpaceEngineRegistry::instance()->get("icc");
                        KIS_ASSERT(engine);
                        file.open(QIODevice::ReadOnly);
                        const QByteArray profileData = file.readAll();
                        if (!profileData.isEmpty()) {
                            profile = engine->addProfile(href);
                        }
                    }
                }


                if (profile && !profiles.contains(name)) {
                    profiles.insert(name, profile);
                }
            }
        }
        for (QDomNode n = doc.documentElement().firstChild(); !n.isNull(); n = n.nextSibling()) {
            QDomElement e = n.toElement();

            if (e.isNull()) continue;

            if (e.tagName() == "linearGradient" || e.tagName() == "radialGradient") {
                parseSvgGradient(e, profiles);
                return;
            }
            // Inkscape gradients are in another defs
            if (e.tagName() == "defs") {


                for (QDomNode defnode = e.firstChild(); !defnode.isNull(); defnode = defnode.nextSibling()) {
                    QDomElement defelement = defnode.toElement();

                    if (defelement.isNull()) continue;

                    if (defelement.tagName() == "linearGradient" || defelement.tagName() == "radialGradient") {
                        parseSvgGradient(defelement, profiles);
                        return;
                    }
                }
            }
        }
    }
}


void KoStopGradient::parseSvgGradient(const QDomElement& element, QHash<QString, const KoColorProfile *> profiles)
{
    m_stops.clear();
    m_hasVariableStops = false;
    setSpread(QGradient::PadSpread);

    /*QString href = e.attribute( "xlink:href" ).mid( 1 );
    if( !href.isEmpty() )
    {
    }*/
    setName(element.attribute("id", i18n("SVG Gradient")));

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
        }
        else {
            m_start = QPointF(element.attribute("x1").toDouble(), element.attribute("y1").toDouble());
            m_stop = QPointF(element.attribute("x2").toDouble(), element.attribute("y2").toDouble());
        }
        setType(QGradient::LinearGradient);
    }
    else {
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
        }
        else {
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
            KoColor color;
            float off;
            QString temp = colorstop.attribute("offset");
            if (temp.contains('%')) {
                temp = temp.left(temp.length() - 1);
                off = temp.toFloat() / 100.0;
            }
            else
                off = temp.toFloat();

            if (!colorstop.attribute("stop-color").isEmpty())
                color = KoColor::fromSVG11(colorstop.attribute("stop-color"), profiles);
            else {
                // try style attr
                QString style = colorstop.attribute("style").simplified();
                QStringList substyles = style.split(';', QString::SkipEmptyParts);
                Q_FOREACH(const QString & s, substyles) {
                    QStringList substyle = s.split(':');
                    QString command = substyle[0].trimmed();
                    QString params = substyle[1].trimmed();
                    if (command == "stop-color")
                        color = KoColor::fromSVG11(params, profiles);
                    if (command == "stop-opacity")
                        opacity = params.toDouble();
                }

            }
            if (!colorstop.attribute("stop-opacity").isEmpty())
                opacity = colorstop.attribute("stop-opacity").toDouble();

            color.setOpacity(static_cast<quint8>(opacity * OPACITY_OPAQUE_U8 + 0.5));
            QString stopTypeStr = colorstop.attribute("krita:stop-type", "color-stop");
            KoGradientStopType stopType = KoGradientStop::typeFromString(stopTypeStr);
            if (stopType != COLORSTOP) {
                m_hasVariableStops = true;
            }
            //According to the SVG spec each gradient offset has to be equal to or greater than the previous one
            //if not it needs to be adjusted to be equal
            if (m_stops.count() > 0 && m_stops.last().position >= off) {
                off = m_stops.last().position;
            }
            m_stops.append(KoGradientStop(off, color, stopType));
        }
    }
}

QString KoStopGradient::defaultFileExtension() const
{
    return QString(".svg");
}

void KoStopGradient::toXML(QDomDocument& doc, QDomElement& gradientElt) const
{
    gradientElt.setAttribute("type", "stop");
    for (int s = 0; s < m_stops.size(); s++) {
        KoGradientStop stop = m_stops.at(s);
        QDomElement stopElt = doc.createElement("stop");
        stopElt.setAttribute("offset", KisDomUtils::toString(stop.position));
        stopElt.setAttribute("bitdepth", stop.color.colorSpace()->colorDepthId().id());
        stopElt.setAttribute("alpha", KisDomUtils::toString(stop.color.opacityF()));
        stopElt.setAttribute("stoptype", KisDomUtils::toString(stop.type));
        stop.color.toXML(doc, stopElt);
        gradientElt.appendChild(stopElt);
    }
}

KoStopGradient KoStopGradient::fromXML(const QDomElement& elt)
{
    KoStopGradient gradient;
    QList<KoGradientStop> stops;
    QDomElement stopElt = elt.firstChildElement("stop");
    while (!stopElt.isNull()) {
        qreal offset = KisDomUtils::toDouble(stopElt.attribute("offset", "0.0"));
        QString bitDepth = stopElt.attribute("bitdepth", Integer8BitsColorDepthID.id());
        KoColor color = KoColor::fromXML(stopElt.firstChildElement(), bitDepth);
        color.setOpacity(KisDomUtils::toDouble(stopElt.attribute("alpha", "1.0")));
        KoGradientStopType stoptype = static_cast<KoGradientStopType>(KisDomUtils::toInt(stopElt.attribute("stoptype", "0")));
        stops.append(KoGradientStop(offset, color, stoptype));
        stopElt = stopElt.nextSiblingElement("stop");
    }
    gradient.setStops(stops);
    return gradient;
}

QString KoStopGradient::saveSvgGradient() const
{
    QDomDocument doc;

    doc.setContent(QString("<svg xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:krita=\"%1\" > </svg>").arg(KoXmlNS::krita));

    const QString spreadMethod[3] = {
        QString("pad"),
        QString("reflect"),
        QString("repeat")
    };

    QDomElement gradient = doc.createElement("linearGradient");
    gradient.setAttribute("id", name());
    gradient.setAttribute("gradientUnits", "objectBoundingBox");
    gradient.setAttribute("spreadMethod", spreadMethod[spread()]);

    QHash<QString, const KoColorProfile*> profiles;
    for(const KoGradientStop & stop: m_stops) {
        QDomElement stopEl = doc.createElement("stop");
        stopEl.setAttribute("stop-color", stop.color.toSVG11(&profiles));
        stopEl.setAttribute("offset", QString().setNum(stop.position));
        stopEl.setAttribute("stop-opacity", stop.color.opacityF());
        stopEl.setAttribute("krita:stop-type", stop.typeString());
        gradient.appendChild(stopEl);
    }

    if (profiles.size()>0) {
        QDomElement defs = doc.createElement("defs");
        for (QString key: profiles.keys()) {
            const KoColorProfile * profile = profiles.value(key);

            QDomElement profileEl = doc.createElement("color-profile");
            profileEl.setAttribute("name", key);
            QString val = profile->uniqueId().toHex();
            profileEl.setAttribute("local", val);
            profileEl.setAttribute("xlink:href", profile->fileName());
            defs.appendChild(profileEl);
        }
        doc.documentElement().appendChild(defs);
    }

    doc.documentElement().appendChild(gradient);

    return doc.toString();
}

bool KoStopGradient::saveToDevice(QIODevice* dev) const
{
    QTextStream stream(dev);

    stream << saveSvgGradient();

    KoResource::saveToDevice(dev);

    return true;
}
