/*
 *  Copyright (c) 2008,2011 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 *  Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
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
#include "kis_painting_assistant.h"
#include "kis_coordinates_converter.h"
#include "kis_debug.h"
#include "kis_dom_utils.h"
#include <kis_canvas2.h>
#include "kis_tool.h"
#include "kis_config.h"

#include <KoStore.h>

#include <QGlobalStatic>
#include <QPen>
#include <QPainter>
#include <QPixmapCache>
#include <QDomElement>
#include <QDomDocument>

Q_GLOBAL_STATIC(KisPaintingAssistantFactoryRegistry, s_instance)

struct KisPaintingAssistantHandle::Private {
    QList<KisPaintingAssistant*> assistants;
    char handle_type;
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
    dbgUI << "KisPaintingAssistantHandle ctor";
}

KisPaintingAssistantHandle& KisPaintingAssistantHandle::operator=(const QPointF &  pt)
{
    setX(pt.x());
    setY(pt.y());
    return *this;
}

void KisPaintingAssistantHandle::setType(char type)
{
    d->handle_type = type;
}

char KisPaintingAssistantHandle::handleType() const
{
    return d->handle_type;
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

bool KisPaintingAssistantHandle::containsAssistant(KisPaintingAssistant* assistant) const
{
    return d->assistants.contains(assistant);
}

void KisPaintingAssistantHandle::mergeWith(KisPaintingAssistantHandleSP handle)
{
    if(this->handleType()== HandleType::NORMAL || handle.data()->handleType()== HandleType::SIDE) {
        return;
    }


    Q_FOREACH (KisPaintingAssistant* assistant, handle->d->assistants) {
        if (!assistant->handles().contains(this)) {
            assistant->replaceHandle(handle, this);
        }
    }
}

void KisPaintingAssistantHandle::uncache()
{
    Q_FOREACH (KisPaintingAssistant* assistant, d->assistants) {
        assistant->uncache();
    }
}

struct KisPaintingAssistant::Private {
    Private();
    explicit Private(const Private &rhs);
    KisPaintingAssistantHandleSP reuseOrCreateHandle(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap, KisPaintingAssistantHandleSP origHandle, KisPaintingAssistant *q, bool registerAssistant = true);
    QList<KisPaintingAssistantHandleSP> handles, sideHandles;
    KisPaintingAssistantHandleSP topLeft, bottomLeft, topRight, bottomRight, topMiddle, bottomMiddle, rightMiddle, leftMiddle;

    // share everything except handles between the clones
    struct SharedData {
        QString id;
        QString name;
        bool isSnappingActive;
        bool outlineVisible;
        KisCanvas2* m_canvas = 0;

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

        QColor assistantGlobalColorCache = QColor(Qt::red);     // color to paint with if a custom color is not set

        bool useCustomColor = false;
        QColor assistantCustomColor = KisConfig(true).defaultAssistantsColor();
    };

    QSharedPointer<SharedData> s;
};

KisPaintingAssistant::Private::Private()
    : s(new SharedData)
{
}

KisPaintingAssistant::Private::Private(const Private &rhs)
    : s(rhs.s)
{
}

KisPaintingAssistantHandleSP KisPaintingAssistant::Private::reuseOrCreateHandle(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap, KisPaintingAssistantHandleSP origHandle, KisPaintingAssistant *q, bool registerAssistant)
{
    KisPaintingAssistantHandleSP mappedHandle = handleMap.value(origHandle);
    if (!mappedHandle) {
        if (origHandle) {
            dbgUI << "handle not found in the map, creating a new one...";
            mappedHandle = KisPaintingAssistantHandleSP(new KisPaintingAssistantHandle(*origHandle));
            dbgUI << "done";
            mappedHandle->setType(origHandle->handleType());
            handleMap.insert(origHandle, mappedHandle);
        } else {
            dbgUI << "orig handle is null, not doing anything";
            mappedHandle = KisPaintingAssistantHandleSP();
        }
    }
    if (mappedHandle && registerAssistant) {
        mappedHandle->registerAssistant(q);
    }
    return mappedHandle;
}

bool KisPaintingAssistant::useCustomColor()
{
    return d->s->useCustomColor;
}

void KisPaintingAssistant::setUseCustomColor(bool useCustomColor)
{
    d->s->useCustomColor = useCustomColor;
}

void KisPaintingAssistant::setAssistantCustomColor(QColor color)
{
    d->s->assistantCustomColor = color;
}

QColor KisPaintingAssistant::assistantCustomColor()
{
    return d->s->assistantCustomColor;
}

void KisPaintingAssistant::setAssistantGlobalColorCache(const QColor &color)
{
    d->s->assistantGlobalColorCache = color;
}

QColor KisPaintingAssistant::effectiveAssistantColor() const
{
    return d->s->useCustomColor ? d->s->assistantCustomColor : d->s->assistantGlobalColorCache;
}

KisPaintingAssistant::KisPaintingAssistant(const QString& id, const QString& name) : d(new Private)
{
    d->s->id = id;
    d->s->name = name;
    d->s->isSnappingActive = true;
    d->s->outlineVisible = true;
}

KisPaintingAssistant::KisPaintingAssistant(const KisPaintingAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : d(new Private(*(rhs.d)))
{
    dbgUI << "creating handles...";
    Q_FOREACH (const KisPaintingAssistantHandleSP origHandle, rhs.d->handles) {
        d->handles << d->reuseOrCreateHandle(handleMap, origHandle, this);
    }
    Q_FOREACH (const KisPaintingAssistantHandleSP origHandle, rhs.d->sideHandles) {
        d->sideHandles << d->reuseOrCreateHandle(handleMap, origHandle, this);
    }
#define _REUSE_H(name) d->name = d->reuseOrCreateHandle(handleMap, rhs.d->name, this, /* registerAssistant = */ false)
    _REUSE_H(topLeft);
    _REUSE_H(bottomLeft);
    _REUSE_H(topRight);
    _REUSE_H(bottomRight);
    _REUSE_H(topMiddle);
    _REUSE_H(bottomMiddle);
    _REUSE_H(rightMiddle);
    _REUSE_H(leftMiddle);
#undef _REUSE_H
    dbgUI << "done";
}

bool KisPaintingAssistant::isSnappingActive() const
{
    return d->s->isSnappingActive;
}

void KisPaintingAssistant::setSnappingActive(bool set)
{
    d->s->isSnappingActive = set;
}


void KisPaintingAssistant::drawPath(QPainter& painter, const QPainterPath &path, bool isSnappingOn)
{

    QColor paintingColor = effectiveAssistantColor();

    if (!isSnappingOn) {
        paintingColor.setAlpha(0.2 * paintingColor.alpha());
    }

    painter.save();
    QPen pen_a(paintingColor, 2);
    pen_a.setCosmetic(true);
    painter.setPen(pen_a);
    painter.drawPath(path);
    painter.restore();
}

void KisPaintingAssistant::drawPreview(QPainter& painter, const QPainterPath &path)
{
    painter.save();
    QPen pen_a(effectiveAssistantColor(), 1);
    pen_a.setStyle(Qt::SolidLine);
    pen_a.setCosmetic(true);
    painter.setPen(pen_a);
    painter.drawPath(path);
    painter.restore();
}

void KisPaintingAssistant::initHandles(QList<KisPaintingAssistantHandleSP> _handles)
{
    Q_ASSERT(d->handles.isEmpty());
    d->handles = _handles;
    Q_FOREACH (KisPaintingAssistantHandleSP handle, _handles) {
        handle->registerAssistant(this);
    }
}

KisPaintingAssistant::~KisPaintingAssistant()
{
    Q_FOREACH (KisPaintingAssistantHandleSP handle, d->handles) {
        handle->unregisterAssistant(this);
    }
    if(!d->sideHandles.isEmpty()) {
        Q_FOREACH (KisPaintingAssistantHandleSP handle, d->sideHandles) {
            handle->unregisterAssistant(this);
        }
    }
    delete d;
}

const QString& KisPaintingAssistant::id() const
{
    return d->s->id;
}

const QString& KisPaintingAssistant::name() const
{
    return d->s->name;
}

void KisPaintingAssistant::replaceHandle(KisPaintingAssistantHandleSP _handle, KisPaintingAssistantHandleSP _with)
{
    Q_ASSERT(d->handles.contains(_handle));
    d->handles.replace(d->handles.indexOf(_handle), _with);
    Q_ASSERT(!d->handles.contains(_handle));
    _handle->unregisterAssistant(this);
    _with->registerAssistant(this);
}

void KisPaintingAssistant::addHandle(KisPaintingAssistantHandleSP handle, HandleType type)
{
    Q_ASSERT(!d->handles.contains(handle));
    if (HandleType::SIDE == type) {
        d->sideHandles.append(handle);
    } else {
        d->handles.append(handle);
    }

    handle->registerAssistant(this);
    handle.data()->setType(type);
}

QPointF KisPaintingAssistant::viewportConstrainedEditorPosition(const KisCoordinatesConverter* converter, const QSize editorSize)
{
    QPointF editorDocumentPos = getEditorPosition();
    QPointF editorWidgetPos = converter->documentToWidgetTransform().map(editorDocumentPos);
    QSizeF canvasSize = converter->getCanvasWidgetSize();
    const int padding = 16;

    editorWidgetPos.rx() = qBound(0.0,
                                  editorWidgetPos.x(),
                                  canvasSize.width() - (editorSize.width() + padding));
    editorWidgetPos.ry() = qBound(0.0,
                                  editorWidgetPos.y(),
                                  canvasSize.height() - (editorSize.height() + padding));

    return converter->widgetToDocument(editorWidgetPos);
}

void KisPaintingAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool useCache, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    Q_UNUSED(updateRect);

    Q_UNUSED(previewVisible);

    findPerspectiveAssistantHandleLocation();

    if (!useCache) {
        gc.save();
        drawCache(gc, converter, assistantVisible);
        gc.restore();
        return;
    }

    const QRect bound = boundingRect();
    if (bound.isEmpty()) {
        return;
    }

    const QTransform transform = converter->documentToWidgetTransform();
    const QRect widgetBound = transform.mapRect(bound);

    const QRect paintRect = transform.mapRect(bound).intersected(gc.viewport());
    if (paintRect.isEmpty()) return;

    QPixmap cached;
    bool found = QPixmapCache::find(d->s->cached, &cached);

    if (!(found &&
          d->s->cachedTransform == transform &&
          d->s->cachedRect.translated(widgetBound.topLeft()).contains(paintRect))) {

        const QRect cacheRect = gc.viewport().adjusted(-100, -100, 100, 100).intersected(widgetBound);
        Q_ASSERT(!cacheRect.isEmpty());

        if (cached.isNull() || cached.size() != cacheRect.size()) {
            cached = QPixmap(cacheRect.size());
        }

        cached.fill(Qt::transparent);
        QPainter painter(&cached);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setWindow(cacheRect);
        drawCache(painter, converter, assistantVisible);
        painter.end();
        d->s->cachedTransform = transform;
        d->s->cachedRect = cacheRect.translated(-widgetBound.topLeft());
        d->s->cached = QPixmapCache::insert(cached);
    }

    gc.drawPixmap(paintRect, cached, paintRect.translated(-widgetBound.topLeft() - d->s->cachedRect.topLeft()));


    if (canvas) {
        d->s->m_canvas = canvas;
    }
}

void KisPaintingAssistant::uncache()
{
    d->s->cached = QPixmapCache::Key();
}

QRect KisPaintingAssistant::boundingRect() const
{
    QRectF r;
    Q_FOREACH (KisPaintingAssistantHandleSP h, handles()) {
        r = r.united(QRectF(*h, QSizeF(1,1)));
    }
    return r.adjusted(-2, -2, 2, 2).toAlignedRect();
}

bool KisPaintingAssistant::isAssistantComplete() const
{
    return true;
}

QByteArray KisPaintingAssistant::saveXml(QMap<KisPaintingAssistantHandleSP, int> &handleMap)
{
    QByteArray data;
    QXmlStreamWriter xml(&data);
    xml.writeStartDocument();
    xml.writeStartElement("assistant");
    xml.writeAttribute("type",d->s->id);
    xml.writeAttribute("active", QString::number(d->s->isSnappingActive));
    xml.writeAttribute("useCustomColor", QString::number(d->s->useCustomColor));
    xml.writeAttribute("customColor",  KisDomUtils::qColorToQString(d->s->assistantCustomColor));



    saveCustomXml(&xml); // if any specific assistants have custom XML data to save to

    // write individual handle data
    xml.writeStartElement("handles");
    Q_FOREACH (const KisPaintingAssistantHandleSP handle, d->handles) {
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

void KisPaintingAssistant::saveCustomXml(QXmlStreamWriter* xml)
{
    Q_UNUSED(xml);
}

void KisPaintingAssistant::loadXml(KoStore* store, QMap<int, KisPaintingAssistantHandleSP> &handleMap, QString path)
{
    int id = 0;
    double x = 0.0, y = 0.0;
    store->open(path);
    QByteArray data = store->read(store->size());
    QXmlStreamReader xml(data);
    while (!xml.atEnd()) {
        switch (xml.readNext()) {
        case QXmlStreamReader::StartElement:
            if (xml.name() == "assistant") {

                QStringRef active = xml.attributes().value("active");
                setSnappingActive( (active != "0")  );

                // load custom shared assistant properties
                if ( xml.attributes().hasAttribute("useCustomColor")) {
                    QStringRef useCustomColor = xml.attributes().value("useCustomColor");

                    bool usingColor = false;
                    if (useCustomColor.toString() == "1") {
                        usingColor = true;
                    }


                    setUseCustomColor(usingColor);
                }

                if ( xml.attributes().hasAttribute("customColor")) {
                    QStringRef customColor = xml.attributes().value("customColor");
                    setAssistantCustomColor( KisDomUtils::qStringToQColor(customColor.toString()) );

                }

            }

            loadCustomXml(&xml);

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
                addHandle(handleMap.value(id), HandleType::NORMAL);
            }
            break;
        default:
            break;
        }
    }
    store->close();
}

bool KisPaintingAssistant::loadCustomXml(QXmlStreamReader* xml)
{
    Q_UNUSED(xml);
    return true;
}

void KisPaintingAssistant::saveXmlList(QDomDocument& doc, QDomElement& assistantsElement,int count)
{
    if (d->s->id == "ellipse"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "ellipse");
        assistantElement.setAttribute("filename", QString("ellipse%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
    else if (d->s->id == "spline"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "spline");
        assistantElement.setAttribute("filename", QString("spline%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
    else if (d->s->id == "perspective"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "perspective");
        assistantElement.setAttribute("filename", QString("perspective%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
    else if (d->s->id == "vanishing point"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "vanishing point");
        assistantElement.setAttribute("filename", QString("vanishing point%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
    else if (d->s->id == "infinite ruler"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "infinite ruler");
        assistantElement.setAttribute("filename", QString("infinite ruler%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
    else if (d->s->id == "parallel ruler"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "parallel ruler");
        assistantElement.setAttribute("filename", QString("parallel ruler%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
    else if (d->s->id == "concentric ellipse"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "concentric ellipse");
        assistantElement.setAttribute("filename", QString("concentric ellipse%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
    else if (d->s->id == "fisheye-point"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "fisheye-point");
        assistantElement.setAttribute("filename", QString("fisheye-point%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
    else if (d->s->id == "ruler"){
        QDomElement assistantElement = doc.createElement("assistant");
        assistantElement.setAttribute("type", "ruler");
        assistantElement.setAttribute("filename", QString("ruler%1.assistant").arg(count));
        assistantsElement.appendChild(assistantElement);
    }
}

void KisPaintingAssistant::findPerspectiveAssistantHandleLocation() {
    QList<KisPaintingAssistantHandleSP> hHandlesList;
    QList<KisPaintingAssistantHandleSP> vHandlesList;
    uint vHole = 0,hHole = 0;
    KisPaintingAssistantHandleSP oppHandle;
    if (d->handles.size() == 4 && d->s->id == "perspective") {
        //get the handle opposite to the first handle
        oppHandle = oppHandleOne();
        //Sorting handles into two list, X sorted and Y sorted into hHandlesList and vHandlesList respectively.
        Q_FOREACH (const KisPaintingAssistantHandleSP handle,d->handles) {
            hHandlesList.append(handle);
            hHole = hHandlesList.size() - 1;
            vHandlesList.append(handle);
            vHole = vHandlesList.size() - 1;
            /*
             sort handles on the basis of X-coordinate
             */
            while(hHole > 0 && hHandlesList.at(hHole -1).data()->x() > handle.data()->x()) {
                hHandlesList.swap(hHole-1, hHole);
                hHole = hHole - 1;
            }
            /*
             sort handles on the basis of Y-coordinate
             */
            while(vHole > 0 && vHandlesList.at(vHole -1).data()->y() > handle.data()->y()) {
                vHandlesList.swap(vHole-1, vHole);
                vHole = vHole - 1;
            }
        }

        /*
         give the handles their respective positions
         */
        if(vHandlesList.at(0).data()->x() > vHandlesList.at(1).data()->x()) {
            d->topLeft = vHandlesList.at(1);
            d->topRight= vHandlesList.at(0);
        }
        else {
            d->topLeft = vHandlesList.at(0);
            d->topRight = vHandlesList.at(1);
        }
        if(vHandlesList.at(2).data()->x() > vHandlesList.at(3).data()->x()) {
            d->bottomLeft = vHandlesList.at(3);
            d->bottomRight = vHandlesList.at(2);
        }
        else {
            d->bottomLeft= vHandlesList.at(2);
            d->bottomRight = vHandlesList.at(3);
        }

        /*
         find if the handles that should be opposite are actually oppositely positioned
         */
        if (( (d->topLeft == d->handles.at(0).data() && d->bottomRight == oppHandle) ||
              (d->topLeft == oppHandle && d->bottomRight == d->handles.at(0).data()) ||
              (d->topRight == d->handles.at(0).data() && d->bottomLeft == oppHandle) ||
              (d->topRight == oppHandle && d->bottomLeft == d->handles.at(0).data()) ) )
        {}
        else {
            if(hHandlesList.at(0).data()->y() > hHandlesList.at(1).data()->y()) {
                d->topLeft = hHandlesList.at(1);
                d->bottomLeft= hHandlesList.at(0);
            }
            else {
                d->topLeft = hHandlesList.at(0);
                d->bottomLeft = hHandlesList.at(1);
            }
            if(hHandlesList.at(2).data()->y() > hHandlesList.at(3).data()->y()) {
                d->topRight = hHandlesList.at(3);
                d->bottomRight = hHandlesList.at(2);
            }
            else {
                d->topRight= hHandlesList.at(2);
                d->bottomRight = hHandlesList.at(3);
            }

        }
        /*
         Setting the middle handles as needed
         */
        if(!d->bottomMiddle && !d->topMiddle && !d->leftMiddle && !d->rightMiddle) {
            d->bottomMiddle = new KisPaintingAssistantHandle((d->bottomLeft.data()->x() + d->bottomRight.data()->x())*0.5,
                                                             (d->bottomLeft.data()->y() + d->bottomRight.data()->y())*0.5);
            d->topMiddle = new KisPaintingAssistantHandle((d->topLeft.data()->x() + d->topRight.data()->x())*0.5,
                                                          (d->topLeft.data()->y() + d->topRight.data()->y())*0.5);
            d->rightMiddle= new KisPaintingAssistantHandle((d->topRight.data()->x() + d->bottomRight.data()->x())*0.5,
                                                           (d->topRight.data()->y() + d->bottomRight.data()->y())*0.5);
            d->leftMiddle= new KisPaintingAssistantHandle((d->bottomLeft.data()->x() + d->topLeft.data()->x())*0.5,
                                                          (d->bottomLeft.data()->y() + d->topLeft.data()->y())*0.5);
            addHandle(d->rightMiddle.data(), HandleType::SIDE);
            addHandle(d->leftMiddle.data(), HandleType::SIDE);
            addHandle(d->bottomMiddle.data(), HandleType::SIDE);
            addHandle(d->topMiddle.data(), HandleType::SIDE);
        }
        else
        {
            d->bottomMiddle.data()->operator =(QPointF((d->bottomLeft.data()->x() + d->bottomRight.data()->x())*0.5,
                                                       (d->bottomLeft.data()->y() + d->bottomRight.data()->y())*0.5));
            d->topMiddle.data()->operator =(QPointF((d->topLeft.data()->x() + d->topRight.data()->x())*0.5,
                                                    (d->topLeft.data()->y() + d->topRight.data()->y())*0.5));
            d->rightMiddle.data()->operator =(QPointF((d->topRight.data()->x() + d->bottomRight.data()->x())*0.5,
                                                      (d->topRight.data()->y() + d->bottomRight.data()->y())*0.5));
            d->leftMiddle.data()->operator =(QPointF((d->bottomLeft.data()->x() + d->topLeft.data()->x())*0.5,
                                                     (d->bottomLeft.data()->y() + d->topLeft.data()->y())*0.5));
        }

    }
}

KisPaintingAssistantHandleSP KisPaintingAssistant::oppHandleOne()
{
    QPointF intersection(0,0);
    if((QLineF(d->handles.at(0).data()->toPoint(),d->handles.at(1).data()->toPoint()).intersect(QLineF(d->handles.at(2).data()->toPoint(),d->handles.at(3).data()->toPoint()), &intersection) != QLineF::NoIntersection)
            && (QLineF(d->handles.at(0).data()->toPoint(),d->handles.at(1).data()->toPoint()).intersect(QLineF(d->handles.at(2).data()->toPoint(),d->handles.at(3).data()->toPoint()), &intersection) != QLineF::UnboundedIntersection))
    {
        return d->handles.at(1);
    }
    else if((QLineF(d->handles.at(0).data()->toPoint(),d->handles.at(2).data()->toPoint()).intersect(QLineF(d->handles.at(1).data()->toPoint(),d->handles.at(3).data()->toPoint()), &intersection) != QLineF::NoIntersection)
            && (QLineF(d->handles.at(0).data()->toPoint(),d->handles.at(2).data()->toPoint()).intersect(QLineF(d->handles.at(1).data()->toPoint(),d->handles.at(3).data()->toPoint()), &intersection) != QLineF::UnboundedIntersection))
    {
        return d->handles.at(2);
    }
    else
    {
        return d->handles.at(3);
    }
}

KisPaintingAssistantHandleSP KisPaintingAssistant::topLeft()
{
    return d->topLeft;
}

const KisPaintingAssistantHandleSP KisPaintingAssistant::topLeft() const
{
    return d->topLeft;
}

KisPaintingAssistantHandleSP KisPaintingAssistant::bottomLeft()
{
    return d->bottomLeft;
}

const KisPaintingAssistantHandleSP KisPaintingAssistant::bottomLeft() const
{
    return d->bottomLeft;
}

KisPaintingAssistantHandleSP KisPaintingAssistant::topRight()
{
    return d->topRight;
}

const KisPaintingAssistantHandleSP KisPaintingAssistant::topRight() const
{
    return d->topRight;
}

KisPaintingAssistantHandleSP KisPaintingAssistant::bottomRight()
{
    return d->bottomRight;
}

const KisPaintingAssistantHandleSP KisPaintingAssistant::bottomRight() const
{
    return d->bottomRight;
}

KisPaintingAssistantHandleSP KisPaintingAssistant::topMiddle()
{
    return d->topMiddle;
}

const KisPaintingAssistantHandleSP KisPaintingAssistant::topMiddle() const
{
    return d->topMiddle;
}

KisPaintingAssistantHandleSP KisPaintingAssistant::bottomMiddle()
{
    return d->bottomMiddle;
}

const KisPaintingAssistantHandleSP KisPaintingAssistant::bottomMiddle() const
{
    return d->bottomMiddle;
}

KisPaintingAssistantHandleSP KisPaintingAssistant::rightMiddle()
{
    return d->rightMiddle;
}

const KisPaintingAssistantHandleSP KisPaintingAssistant::rightMiddle() const
{
    return d->rightMiddle;
}

KisPaintingAssistantHandleSP KisPaintingAssistant::leftMiddle()
{
    return d->leftMiddle;
}

const KisPaintingAssistantHandleSP KisPaintingAssistant::leftMiddle() const
{
    return d->leftMiddle;
}

const QList<KisPaintingAssistantHandleSP>& KisPaintingAssistant::handles() const
{
    return d->handles;
}

QList<KisPaintingAssistantHandleSP> KisPaintingAssistant::handles()
{
    return d->handles;
}

const QList<KisPaintingAssistantHandleSP>& KisPaintingAssistant::sideHandles() const
{
    return d->sideHandles;
}

QList<KisPaintingAssistantHandleSP> KisPaintingAssistant::sideHandles()
{
    return d->sideHandles;
}



bool KisPaintingAssistant::areTwoPointsClose(const QPointF& pointOne, const QPointF& pointTwo)
{
    int m_handleSize = 16;

    QRectF handlerect(pointTwo - QPointF(m_handleSize * 0.5, m_handleSize * 0.5), QSizeF(m_handleSize, m_handleSize));
    return handlerect.contains(pointOne);
}

KisPaintingAssistantHandleSP KisPaintingAssistant::closestCornerHandleFromPoint(QPointF point)
{
    if (!d->s->m_canvas) {
        return 0;
    }


    if (areTwoPointsClose(point, pixelToView(topLeft()->toPoint()))) {
        return topLeft();
    } else if (areTwoPointsClose(point, pixelToView(topRight()->toPoint()))) {
        return topRight();
    } else if (areTwoPointsClose(point, pixelToView(bottomLeft()->toPoint()))) {
        return bottomLeft();
    } else if (areTwoPointsClose(point, pixelToView(bottomRight()->toPoint()))) {
        return bottomRight();
    }
    return 0;
}


QPointF KisPaintingAssistant::pixelToView(const QPoint pixelCoords) const
{
    QPointF documentCoord = d->s->m_canvas->image()->pixelToDocument(pixelCoords);
    return d->s->m_canvas->viewConverter()->documentToView(documentCoord);
}

double KisPaintingAssistant::norm2(const QPointF& p)
{
    return p.x() * p.x() + p.y() * p.y();
}

QList<KisPaintingAssistantSP> KisPaintingAssistant::cloneAssistantList(const QList<KisPaintingAssistantSP> &list)
{
    QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> handleMap;
    QList<KisPaintingAssistantSP> clonedList;
    for (auto i = list.begin(); i != list.end(); ++i) {
        clonedList << (*i)->clone(handleMap);
    }
    return clonedList;
}



/*
 * KisPaintingAssistantFactory classes
*/

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
    Q_FOREACH (const QString &id, keys()) {
        delete get(id);
    }
    dbgRegistry << "deleting KisPaintingAssistantFactoryRegistry ";
}

KisPaintingAssistantFactoryRegistry* KisPaintingAssistantFactoryRegistry::instance()
{
    return s_instance;
}

