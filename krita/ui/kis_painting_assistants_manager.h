/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_PAINTING_ASSISTANTS_MANAGER_H_
#define _KIS_PAINTING_ASSISTANTS_MANAGER_H_

#include <QPointF>

#include "canvas/kis_canvas_decoration.h"
#include "kis_painting_assistant.h"

#include <krita_export.h>

class KActionCollection;

/**
 * This class hold a list of painting assistants.
 */
class KRITAUI_EXPORT KisPaintingAssistantsManager : public KisCanvasDecoration
{
public:
    KisPaintingAssistantsManager(KisView2* parent);
    ~KisPaintingAssistantsManager();
    void addAssistant(KisPaintingAssistant* assistant);
    void removeAssistant(KisPaintingAssistant* assistant);
    QPointF adjustPosition(const QPointF& point) const;
    void setup(KActionCollection * collection);
    QList<KisPaintingAssistantHandleSP> handles();
protected:
    virtual void drawDecoration(QPainter& gc, const QPoint& documentOffset,  const QRect& area, const KoViewConverter &converter);
private:
    struct Private;
    Private* const d;
};

#endif
