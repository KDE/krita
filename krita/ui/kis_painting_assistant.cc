/*
 *  Copyright (c) 2008,2011 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "kis_painting_assistant.h"
#include "kis_coordinates_converter.h"
#include "kis_debug.h"

#include <kglobal.h>
#include <QPen>
#include <QPainter>
#include <QPixmapCache>

struct KisPaintingAssistantHandle::Private {
    QList<KisPaintingAssistant*> assistants;
};

KisPaintingAssistantHandle::KisPaintingAssistantHandle(double x, double y) : QPointF(x, y), d(new Private)
{
}
KisPaintingAssistantHandle::KisPaintingAssistantHandle(QPointF p) : QPointF(p), d(new Private)
{
}

KisPaintingAssistantHandle::KisPaintingAssistantHandle(const KisPaintingAssistantHandle& rhs)
    : QPointF(rhs)
    , KisShared()
    , d(new Private)
{
}

KisPaintingAssistantHandle& KisPaintingAssistantHandle::operator=(const QPointF &  pt)
{
    setX(pt.x());
    setY(pt.y());
    return *this;
}

KisPaintingAssistantHandle::~KisPaintingAssistantHandle()
{
    Q_ASSERT(d->assistants.empty());
    delete d;
}

void KisPaintingAssistantHandle::registerAssistant(KisPaintingAssistant* assistant)
{
    Q_ASSERT(!d->assistants.contains(assistant));
    d->assistants.append(assistant);
}

void KisPaintingAssistantHandle::unregisterAssistant(KisPaintingAssistant* assistant)
{
    d->assistants.removeOne(assistant);
    Q_ASSERT(!d->assistants.contains(assistant));
}

bool KisPaintingAssistantHandle::containsAssistant(KisPaintingAssistant* assistant)
{
    return d->assistants.contains(assistant);
}

void KisPaintingAssistantHandle::mergeWith(KisPaintingAssistantHandleSP handle)
{
    foreach(KisPaintingAssistant* assistant, handle->d->assistants) {
        if (!assistant->handles().contains(this)) {
            assistant->replaceHandle(handle, this);
        }
    }
}

QList<KisPaintingAssistantHandleSP> KisPaintingAssistantHandle::split()
{
    QList<KisPaintingAssistantHandleSP> newHandles;
    foreach(KisPaintingAssistant* assistant, d->assistants) {
        KisPaintingAssistantHandleSP newHandle(new KisPaintingAssistantHandle(*this));
        newHandles.append(newHandle);
        assistant->replaceHandle(this, newHandle);
    }
    return newHandles;
}

void KisPaintingAssistantHandle::uncache()
{
    foreach(KisPaintingAssistant* assistant, d->assistants) {
        assistant->uncache();
    }
}


struct KisPaintingAssistant::Private {
    QString id;
    QString name;
    QList<KisPaintingAssistantHandleSP> handles;
    QPixmapCache::Key cached;
    QRect cachedRect; // relative to boundingRect().topLeft()
    struct TranslationInvariantTransform {
        qreal m11, m12, m21, m22;
        TranslationInvariantTransform() { }
        TranslationInvariantTransform(const QTransform& t) : m11(t.m11()), m12(t.m12()), m21(t.m21()), m22(t.m22()) { }
        bool operator==(const TranslationInvariantTransform& b) {
            return m11 == b.m11 && m12 == b.m12 && m21 == b.m21 && m22 == b.m22;
        }
    } cachedTransform;
};

KisPaintingAssistant::KisPaintingAssistant(const QString& id, const QString& name) : d(new Private)
{
    d->id = id;
    d->name = name;
}

void KisPaintingAssistant::drawPath(QPainter& painter, const QPainterPath &path)
{
    painter.save();
    QPen pen_a(QColor(0, 0, 0, 100), 2);
    pen_a.setCosmetic(true);
    painter.setPen(pen_a);
    painter.drawPath(path);
    QPen pen_b(Qt::white, 0.9);
    pen_b.setCosmetic(true);
    painter.setPen(pen_b);
    painter.drawPath(path);
    painter.restore();
}

void KisPaintingAssistant::initHandles(QList<KisPaintingAssistantHandleSP> _handles)
{
    Q_ASSERT(d->handles.isEmpty());
    d->handles = _handles;
    foreach(KisPaintingAssistantHandleSP handle, _handles) {
        handle->registerAssistant(this);
    }
}

KisPaintingAssistant::~KisPaintingAssistant()
{
    foreach(KisPaintingAssistantHandleSP handle, d->handles) {
        handle->unregisterAssistant(this);
    }
    delete d;
}

const QString& KisPaintingAssistant::id() const
{
    return d->id;
}

const QString& KisPaintingAssistant::name() const
{
    return d->name;
}

void KisPaintingAssistant::replaceHandle(KisPaintingAssistantHandleSP _handle, KisPaintingAssistantHandleSP _with)
{
    Q_ASSERT(d->handles.contains(_handle));
    d->handles.replace(d->handles.indexOf(_handle), _with);
    Q_ASSERT(!d->handles.contains(_handle));
    _handle->unregisterAssistant(this);
    _with->registerAssistant(this);
}

void KisPaintingAssistant::addHandle(KisPaintingAssistantHandleSP handle)
{
    Q_ASSERT(!d->handles.contains(handle));
    d->handles.append(handle);
    handle->registerAssistant(this);
}

void KisPaintingAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool useCache)
{
    Q_UNUSED(updateRect);

    if (!useCache) {
        gc.save();
        drawCache(gc, converter);
        gc.restore();
        return;
    }
    const QRect bound = boundingRect();
    if (bound.isEmpty()) return;
    const QTransform transform = converter->documentToWidgetTransform();
    const QRect widgetBound = transform.mapRect(bound);

    const QRect paintRect = transform.mapRect(bound).intersected(gc.viewport());
    if (paintRect.isEmpty()) return;

    QPixmap cached;
    if (!(QPixmapCache::find(d->cached, &cached) &&
          d->cachedTransform == transform &&
          d->cachedRect.translated(widgetBound.topLeft()).contains(paintRect))) {
        const QRect cacheRect = gc.viewport().adjusted(-100, -100, 100, 100).intersected(widgetBound);
        Q_ASSERT(!cacheRect.isEmpty());
        if (cached.isNull() || cached.size() != cacheRect.size()) {
            cached = QPixmap(cacheRect.size());
        }
        cached.fill(Qt::transparent);
        QPainter painter(&cached);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setWindow(cacheRect);
        drawCache(painter, converter);
        painter.end();
        d->cachedTransform = transform;
        d->cachedRect = cacheRect.translated(-widgetBound.topLeft());
        d->cached = QPixmapCache::insert(cached);
    }
    gc.drawPixmap(paintRect, cached, paintRect.translated(-widgetBound.topLeft() - d->cachedRect.topLeft()));
}

void KisPaintingAssistant::uncache()
{
    d->cached = QPixmapCache::Key();
}

QRect KisPaintingAssistant::boundingRect() const
{
    QRectF r;
    foreach (KisPaintingAssistantHandleSP h, handles()) {
        r = r.united(QRectF(*h, QSizeF(1,1)));
    }
    return r.adjusted(-2, -2, 2, 2).toAlignedRect();
}

QByteArray KisPaintingAssistant::saveXml(QMap<KisPaintingAssistantHandleSP, int> &handleMap)
{
        QByteArray data;
        QXmlStreamWriter xml(&data);
            xml.writeStartDocument();
            xml.writeStartElement("assistant");
            xml.writeAttribute("type",d->id);
            xml.writeStartElement("handles");
            foreach(const KisPaintingAssistantHandleSP handle, d->handles) {
                int id = handleMap.size();
                if (!handleMap.contains(handle)){
                    handleMap.insert(handle, id);
                }
                id = handleMap.value(handle);
                xml.writeStartElement("handle");
                xml.writeAttribute("id", QString::number(id));
                xml.writeAttribute("x", QString::number(double(handle->x()), 'f', 3));
                xml.writeAttribute("y", QString::number(double(handle->y()), 'f', 3));
                xml.writeEndElement();
            }
            xml.writeEndElement();
            xml.writeEndElement();
            xml.writeEndDocument();
            return data;
}

void KisPaintingAssistant::loadXml(KoStore* store, QMap<int, KisPaintingAssistantHandleSP> &handleMap, QString path)
{
    int id;
    double x,y ;
    store->open(path);
    QByteArray data = store->read(store->size());
    QXmlStreamReader xml(data);
    while (!xml.atEnd()) {
        switch (xml.readNext()) {
        case QXmlStreamReader::StartElement:
            if (xml.name() == "handle") {
                QString strId = xml.attributes().value("id").toString(),
                strX = xml.attributes().value("x").toString(),
                strY = xml.attributes().value("y").toString();
                if (!strId.isEmpty() && !strX.isEmpty() && !strY.isEmpty()) {
                    id = strId.toInt();
                    x = strX.toDouble();
                    y = strY.toDouble();
                    if (!handleMap.contains(id)) {
                        handleMap.insert(id, new KisPaintingAssistantHandle(x, y));
                    }
                }
                addHandle(handleMap.value(id));
            }
            break;
        default:
            break;
        }
    }
    store->close();
}

void KisPaintingAssistant::saveXmlList(QDomDocument& doc, QDomElement& assistantsElement,int count)
{
    if (d->id == "ellipse"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "ellipse");
        assistantElement.setAttribute("filename", QString("ellipse%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
    else if (d->id == "spline"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "spline");
        assistantElement.setAttribute("filename", QString("spline%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
    else if (d->id == "perspective"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "perspective");
        assistantElement.setAttribute("filename", QString("perspective%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
    else if (d->id == "ruler"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "ruler");
        assistantElement.setAttribute("filename", QString("ruler%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
}

const QList<KisPaintingAssistantHandleSP>& KisPaintingAssistant::handles() const
{
    return d->handles;
}

QList<KisPaintingAssistantHandleSP> KisPaintingAssistant::handles()
{
    return d->handles;
}

KisPaintingAssistantFactory::KisPaintingAssistantFactory()
{
}

KisPaintingAssistantFactory::~KisPaintingAssistantFactory()
{
}

KisPaintingAssistantFactoryRegistry::KisPaintingAssistantFactoryRegistry()
{
}

KisPaintingAssistantFactoryRegistry::~KisPaintingAssistantFactoryRegistry()
{
    foreach(QString id, keys()) {
        delete get(id);
    }
    dbgRegistry << "deleting KisPaintingAssistantFactoryRegistry ";
}

KisPaintingAssistantFactoryRegistry* KisPaintingAssistantFactoryRegistry::instance()
{
    K_GLOBAL_STATIC(KisPaintingAssistantFactoryRegistry, s_instance);
    return s_instance;
}

