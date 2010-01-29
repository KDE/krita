/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
 * Copyright (C) 2009 Carlos Licea <carlos.licea@kdemail.net>
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

#ifndef PARAGRAPHTOOL_H
#define PARAGRAPHTOOL_H

#include "ParagraphEditor.h"
#include "ParagraphHighlighter.h"

#include <KoToolBase.h>

class KoCanvasBase;
class KoPointerEvent;
class KoViewConverter;

class QKeyEvent;
class QWidget;

/**
 * This is the tool for editing paragraph formatting
 * It displays all paragraph formatting parameters directly on the canvas
 * and allows to modify them, too.
 */
class ParagraphTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit ParagraphTool(KoCanvasBase *canvas);
    ~ParagraphTool();

    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    virtual void activate(bool temporary = false);
    virtual void deactivate();

    virtual void repaintDecorations();

protected:
    virtual QWidget *createOptionWidget();

private:
    void repaintDecorationsInternal();

    ParagraphEditor m_paragraphEditor;
    ParagraphHighlighter m_paragraphHighlighter;
    QPointF m_mousePosition;
};

#endif

