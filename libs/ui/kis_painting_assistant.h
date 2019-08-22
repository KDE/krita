/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_PAINTING_ASSISTANT_H_
#define _KIS_PAINTING_ASSISTANT_H_

#include <QString>
#include <QPointF>
#include <QRect>
#include <QFile>
#include <QObject>
#include <QColor>
#include <QXmlStreamWriter>
#include <QMap>

#include <kritaui_export.h>
#include <kis_shared.h>
#include <kis_types.h>

class QPainter;
class QRect;
class QRectF;
class KoStore;
class KisCoordinatesConverter;
class KisCanvas2;
class QDomDocument;
class QDomElement;

#include <kis_shared_ptr.h>
#include <KoGenericRegistry.h>

class KisPaintingAssistantHandle;
typedef KisSharedPtr<KisPaintingAssistantHandle> KisPaintingAssistantHandleSP;
class KisPaintingAssistant;
class QPainterPath;

enum HandleType {
    NORMAL,
    SIDE,
    CORNER,
    VANISHING_POINT,
    ANCHOR
};


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
    void uncache();
    KisPaintingAssistantHandle& operator=(const QPointF&);
    void setType(char type);
    char handleType() const;

private:
    void registerAssistant(KisPaintingAssistant*);
    void unregisterAssistant(KisPaintingAssistant*);
    bool containsAssistant(KisPaintingAssistant*) const;

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
    virtual KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const = 0;
    const QString& id() const;
    const QString& name() const;
    bool isSnappingActive() const;
    void setSnappingActive(bool set);


    /**
     * Adjust the position given in parameter.
     * @param point the coordinates in point in the document reference
     * @param strokeBegin the coordinates of the beginning of the stroke
     */
    virtual QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin) = 0;
    virtual void endStroke() { }
    virtual QPointF getEditorPosition() const = 0; // Returns editor widget position in document-space coordinates.
    virtual int numHandles() const = 0;

    void replaceHandle(KisPaintingAssistantHandleSP _handle, KisPaintingAssistantHandleSP _with);
    void addHandle(KisPaintingAssistantHandleSP handle, HandleType type);

    QPointF viewportConstrainedEditorPosition(const KisCoordinatesConverter* converter, const QSize editorSize);

    QColor effectiveAssistantColor() const;
    bool useCustomColor();
    void setUseCustomColor(bool useCustomColor);
    void setAssistantCustomColor(QColor color);
    QColor assistantCustomColor();
    void setAssistantGlobalColorCache(const QColor &color);

    virtual void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, bool cached = true,KisCanvas2 *canvas=0, bool assistantVisible=true, bool previewVisible=true);
    void uncache();
    const QList<KisPaintingAssistantHandleSP>& handles() const;
    QList<KisPaintingAssistantHandleSP> handles();
    const QList<KisPaintingAssistantHandleSP>& sideHandles() const;
    QList<KisPaintingAssistantHandleSP> sideHandles();

    QByteArray saveXml( QMap<KisPaintingAssistantHandleSP, int> &handleMap);
    virtual void saveCustomXml(QXmlStreamWriter* xml); //in case specific assistants have custom properties (like vanishing point)

    void loadXml(KoStore *store, QMap<int, KisPaintingAssistantHandleSP> &handleMap, QString path);
    virtual bool loadCustomXml(QXmlStreamReader* xml);

    void saveXmlList(QDomDocument& doc, QDomElement& ssistantsElement, int count);
    void findPerspectiveAssistantHandleLocation();
    KisPaintingAssistantHandleSP oppHandleOne();

    /**
      * Get the topLeft, bottomLeft, topRight and BottomRight corners of the assistant
      * Some assistants like the perspective grid have custom logic built around certain handles
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


    // calculates whether a point is near one of the corner points of the assistant
    // returns: a corner point from the perspective assistant if the given node is close
    // only called once in code when calculating the perspective assistant
    KisPaintingAssistantHandleSP closestCornerHandleFromPoint(QPointF point);

    // determines if two points are close to each other
    // only used by the nodeNearPoint function (perspective grid assistant).
    bool areTwoPointsClose(const QPointF& pointOne, const QPointF& pointTwo);

    /// determines if the assistant has enough handles to be considered created
    /// new assistants get in a "creation" phase where they are currently being made on the canvas
    /// it will return false if we are in the middle of creating the assistant.
    virtual bool isAssistantComplete() const;

    virtual void transform(const QTransform &transform);

public:
    /**
     * This will render the final output. The drawCache does rendering most of the time so be sure to check that
     */
    void drawPath(QPainter& painter, const QPainterPath& path, bool drawActive=true);
    void drawPreview(QPainter& painter, const QPainterPath& path);
    static double norm2(const QPointF& p);

protected:
    explicit KisPaintingAssistant(const KisPaintingAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);

    virtual QRect boundingRect() const;

    /// performance layer where the graphics can be drawn from a cache instead of generated every render update
    virtual void drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible=true) = 0;

    void initHandles(QList<KisPaintingAssistantHandleSP> _handles);
    QList<KisPaintingAssistantHandleSP> m_handles;

    QPointF pixelToView(const QPoint pixelCoords) const;
public:
    /// clones the list of assistants
    /// the originally shared handles will still be shared
    /// the cloned assistants do not share any handle with the original assistants
    static QList<KisPaintingAssistantSP> cloneAssistantList(const QList<KisPaintingAssistantSP> &list);

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
    ~KisPaintingAssistantFactoryRegistry() override;

    static KisPaintingAssistantFactoryRegistry* instance();

};

#endif
