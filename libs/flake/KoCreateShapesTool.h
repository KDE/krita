/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOCREATESHAPESTOOL_H
#define KOCREATESHAPESTOOL_H

#include "KoInteractionTool.h"
#include "KoShapeFactory.h"

#include <koffice_export.h>

#include <QString>

class KoCanvasBase;
class KoShapeControllerBase;


/**
 * A tool to create shapes with.
 */
class FLAKE_EXPORT KoCreateShapesTool : public KoInteractionTool
{
public:
    KoCreateShapesTool( KoCanvasBase *canvas);
    virtual ~KoCreateShapesTool() {};
    void mouseReleaseEvent( KoPointerEvent *event );

    void paint( QPainter &painter, KoViewConverter &converter );

    KoShapeControllerBase* controller() const { return m_shapeController; }

    void setShapeController(KoShapeControllerBase *sc) { m_shapeController = sc; }

    void setShapeId(QString id) { m_shapeId = id; }
    const QString& shapeId() const { return m_shapeId; }

private:
    KoShapeControllerBase *m_shapeController;
    QString m_shapeId;
};

#endif
