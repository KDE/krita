/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
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

#ifndef COMMENTTOOL_H
#define COMMENTTOOL_H

#include <KoToolBase.h>

class CommentShape;

class CommentTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit CommentTool(KoCanvasBase *canvas);
    virtual ~CommentTool();

    virtual void activate(KoToolBase::ToolActivation toolActivation, const QSet< KoShape* >& shapes);
    virtual void deactivate();

    virtual void mouseReleaseEvent(KoPointerEvent* event);
    virtual void mouseMoveEvent(KoPointerEvent* event);
    virtual void mousePressEvent(KoPointerEvent* event);
    virtual void paint(QPainter& painter, const KoViewConverter& converter);

private:
    KoCanvasBase* m_canvas;
    CommentShape* m_previouseActiveCommentShape;
    bool m_temporary;
};

#endif // COMMENTTOOL_H
