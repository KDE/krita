/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_PAINTING_ASSISTANT_H_
#define _KIS_PAINTING_ASSISTANT_H_

#include <QString>
#include <QPointF>
#include <QRect>
#include <QFile>
#include <QObject>

#include <KisDocument.h>
#include <kritaui_export.h>
#include <kis_shared.h>
#include <kis_canvas2.h>

class QPainter;
class QRect;
class QRectF;
class KoStore;
class KisCoordinatesConverter;

#include <kis_shared_ptr.h>
#include <KoGenericRegistry.h>

class KisPaintingAssistantHandle;
typedef KisSharedPtr<KisPaintingAssistantHandle> KisPaintingAssistantHandleSP;
class KisPaintingAssistant;
class QPainterPath;

/**
  * Represent an handle of the assistant, used to edit the parameters
  * of an assistants. Handles can be shared between assistants.
  */
class KRITAUI_EXPORT KisPaintingAssistantHandle : public QPointF, public KisShared
{
    friend class KisPaintingAssistant;
public:
    KisPaintingAssistantHandle(double x, double y);
    explicit KisPaintingAssistantHandle(QPointF p);
    KisPaintingAssistantHandle(const KisPaintingAssistantHandle&);
    ~KisPaintingAssistantHandle();
    void mergeWith(KisPaintingAssistantHandleSP);
    QList<KisPaintingAssistantHandleSP> split();
    void uncache();
    KisPaintingAssistantHandle& operator=(const QPointF&);
    void setType(char type);
    char handleType();
private:
    void registerAssistant(KisPaintingAssistant*);
    void unregisterAssistant(KisPaintingAssistant*);
    bool containsAssistant(KisPaintingAssistant*);
private:
    struct Private;
    Private* const d;
};

/**
 * A KisPaintingAssistant is an object that assist the drawing on the canvas.
 * With this class you can implement virtual equivalent to ruler or compas.
 */
class KRITAUI_EXPORT KisPaintingAssistant
{
public:
    KisPaintingAssistant(const QString& id, const QString& name);
    virtual ~KisPaintingAssistant();
    const QString& id() const;
    const QString& name() const;
    bool snapping() const;//this returns whether or not the snapping is/should be active.
    void setSnapping(bool set);
    bool outline() const;//this returns whether or not the preview is/should be active.
    void setOutline(bool set);
    /**
     * Adjust the position given in parameter.
     * @param point the coordinates in point in the document reference
     * @param strokeBegin the coordinates of the beginning of the stroke
     */
    virtual QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin) = 0;
    virtual void endStroke() { }
    virtual QPointF buttonPosition() const = 0;
    virtual int numHandles() const = 0;
    void replaceHandle(KisPaintingAssistantHandleSP _handle, KisPaintingAssistantHandleSP _with);
    void addHandle(KisPaintingAssistantHandleSP handle);
    void addSideHandle(KisPaintingAssistantHandleSP handle);
    virtual void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, bool cached = true,KisCanvas2 *canvas=0, bool assistantVisible=true, bool previewVisible=true);
    void uncache();
    const QList<KisPaintingAssistantHandleSP>& handles() const;
    QList<KisPaintingAssistantHandleSP> handles();
    const QList<KisPaintingAssistantHandleSP>& sideHandles() const;
    QList<KisPaintingAssistantHandleSP> sideHandles();
    QByteArray saveXml( QMap<KisPaintingAssistantHandleSP, int> &handleMap);
    void loadXml(KoStore *store, QMap<int, KisPaintingAssistantHandleSP> &handleMap, QString path);
    void saveXmlList(QDomDocument& doc, QDomElement& ssistantsElement, int count);
    void findHandleLocation();
    KisPaintingAssistantHandleSP oppHandleOne();

    /**
      * Get the topLeft, bottomLeft, topRight and BottomRight corners of the assistant
      */
    const KisPaintingAssistantHandleSP topLeft() const;
    KisPaintingAssistantHandleSP topLeft();
    const KisPaintingAssistantHandleSP topRight() const;
    KisPaintingAssistantHandleSP topRight();
    const KisPaintingAssistantHandleSP bottomLeft() const;
    KisPaintingAssistantHandleSP bottomLeft();
    const KisPaintingAssistantHandleSP bottomRight() const;
    KisPaintingAssistantHandleSP bottomRight();
    const KisPaintingAssistantHandleSP topMiddle() const;
    KisPaintingAssistantHandleSP topMiddle();
    const KisPaintingAssistantHandleSP rightMiddle() const;
    KisPaintingAssistantHandleSP rightMiddle();
    const KisPaintingAssistantHandleSP leftMiddle() const;
    KisPaintingAssistantHandleSP leftMiddle();
    const KisPaintingAssistantHandleSP bottomMiddle() const;
    KisPaintingAssistantHandleSP bottomMiddle();

public:
    /**
     * This will paint a path using a white and black colors.
     */
    static void drawPath(QPainter& painter, const QPainterPath& path, bool drawActive=true);
    static void drawPreview(QPainter& painter, const QPainterPath& path);
protected:
    virtual QRect boundingRect() const;
    virtual void drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible=true) = 0;
    void initHandles(QList<KisPaintingAssistantHandleSP> _handles);
    QList<KisPaintingAssistantHandleSP> m_handles;
private:
    struct Private;
    Private* const d;
    
};

/**
 * Allow to create a painting assistant.
 */
class KRITAUI_EXPORT KisPaintingAssistantFactory
{
public:
    KisPaintingAssistantFactory();
    virtual ~KisPaintingAssistantFactory();
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual KisPaintingAssistant* createPaintingAssistant() const = 0;

};

class KRITAUI_EXPORT KisPaintingAssistantFactoryRegistry : public KoGenericRegistry<KisPaintingAssistantFactory*>
{
  public:
    KisPaintingAssistantFactoryRegistry();
    ~KisPaintingAssistantFactoryRegistry();

    static KisPaintingAssistantFactoryRegistry* instance();
  
};

#endif
