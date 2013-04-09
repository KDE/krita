/* This file is part of the KDE project
 * Copyright (C) 2009-2010 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "ReviewTool.h"
#include <KoToolBase.h>
#include <KoCanvasBase.h>
#include <KoPointerEvent.h>

#include <dialogs/SimpleSpellCheckingWidget.h>

//#include "TextShape.h"

ReviewTool::ReviewTool(KoCanvasBase* canvas): TextTool(canvas),
    m_textEditor(0),
    m_textShapeData(0),
    m_canvas(canvas),
    m_textShape(0)
{
    createActions();
}

ReviewTool::~ReviewTool()
{
}

void ReviewTool::createActions()
{

}

void ReviewTool::mouseReleaseEvent(KoPointerEvent* event)
{
    TextTool::mouseReleaseEvent(event);
}
void ReviewTool::activate(KoToolBase::ToolActivation toolActivation, const QSet< KoShape* >& shapes)
{
    TextTool::activate(toolActivation, shapes);
}
void ReviewTool::deactivate()
{
    TextTool::deactivate();
}
void ReviewTool::mouseMoveEvent(KoPointerEvent* event)
{
    TextTool::mouseMoveEvent(event);
}
void ReviewTool::mousePressEvent(KoPointerEvent* event)
{
    TextTool::mousePressEvent(event);
}
void ReviewTool::keyPressEvent(QKeyEvent* event)
{
    TextTool::keyPressEvent(event);
}
void ReviewTool::paint(QPainter& painter, const KoViewConverter& converter)
{
    TextTool::paint(painter,converter);
}

QList<QWidget *> ReviewTool::createOptionWidgets()
{
    QList<QWidget *> widgets;
    SimpleSpellCheckingWidget* sscw = new SimpleSpellCheckingWidget(this, 0);
    sscw->setWindowTitle("SpellCheck");
    widgets.append(sscw);
    return widgets;

}
