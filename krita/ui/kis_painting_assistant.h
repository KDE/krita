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

#include <krita_export.h>
#include <kis_shared.h>

class QPainter;
class QRect;
class KoViewConverter;

#include <kis_shared_ptr.h>

class KisPaintingAssistantHandle;
typedef KisSharedPtr<KisPaintingAssistantHandle> KisPaintingAssistantHandleSP;
class KisPaintingAssistant;


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
    KisPaintingAssistantHandle& operator=(const QPointF&);
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
    /**
     * Adjust the position given in parameter.
     * @param point the coordinates in point in the document reference
     */
    virtual QPointF adjustPosition(const QPointF& point) const = 0;
    virtual void drawAssistant(QPainter& gc, const QPoint& documentOffset,  const QRect& area, const KoViewConverter &converter) const = 0;
    void replaceHandle(KisPaintingAssistantHandleSP _handle, KisPaintingAssistantHandleSP _with);
    const QList<KisPaintingAssistantHandleSP>& handles() const;
    QList<KisPaintingAssistantHandleSP> handles();
protected:
    void initHandles(QList<KisPaintingAssistantHandleSP> _handles);
private:
    struct Private;
    Private* const d;
};

#endif
