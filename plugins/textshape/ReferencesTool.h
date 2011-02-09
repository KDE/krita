/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
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

#ifndef REFERENCESTOOL_H
#define REFERENCESTOOL_H

#include <KoToolBase.h>

class KoCanvasBase;
class QPainter;

/// This tool is the ui for inserting Table of Contents, citatons /bibliography, endnotes, index, table of illustrations etc

class ReferencesTool : public KoToolBase
{
    Q_OBJECT
public:
    ReferencesTool(KoCanvasBase *canvas);

    ~ReferencesTool();

    virtual void mouseReleaseEvent(KoPointerEvent* event);
    virtual void mouseMoveEvent(KoPointerEvent* event);
    virtual void mousePressEvent(KoPointerEvent* event);
    virtual void paint(QPainter& painter, const KoViewConverter& converter);
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    virtual void deactivate();

protected:
    virtual QWidget* createOptionWidget();

private:
    KoCanvasBase *m_canvas;
};

#endif // REFERENCESTOOL_H
