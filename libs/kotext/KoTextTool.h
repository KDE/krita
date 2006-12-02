/* This file is part of the KDE project
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

#ifndef KOTEXTTOOL_H
#define KOTEXTTOOL_H

#include "KoTextShape.h"
#include "KoTextSelectionHandler.h"

#include <KoTool.h>

#include <QTextCursor>


/**
 * This is the tool for the text-shape (which is a flake-based plugin).
 */
class KoTextTool : public KoTool {
    Q_OBJECT
public:
    explicit KoTextTool(KoCanvasBase *canvas);
    ~KoTextTool();

    void paint( QPainter &painter, KoViewConverter &converter );

    void mousePressEvent( KoPointerEvent *event ) ;
    void mouseDoubleClickEvent( KoPointerEvent *event );
    void mouseMoveEvent( KoPointerEvent *event );
    void mouseReleaseEvent( KoPointerEvent *event );
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void activate (bool temporary=false);
    void deactivate();

    KoToolSelection* selection();

private:
    void repaint();
    int pointToPosition(const QPointF & point) const;
    void updateSelectionHandler();

private:
    KoTextShape *m_textShape;
    KoTextShapeData *m_textShapeData;
    QTextCursor m_caret;
    KoTextSelectionHandler m_selectionHandler;
};

#endif
