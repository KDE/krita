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

#include "ReferencesTool.h"

#include <KoCanvasBase.h>
#include "TextShape.h"

#include <KLocale>

ReferencesTool::ReferencesTool(KoCanvasBase* canvas): KoToolBase(canvas),
    m_canvas(canvas)
{
}

ReferencesTool::~ReferencesTool()
{
}


void ReferencesTool::mouseReleaseEvent(KoPointerEvent* event)
{
}

void ReferencesTool::mouseMoveEvent(KoPointerEvent* event)
{
}

void ReferencesTool::mousePressEvent(KoPointerEvent* event)
{
}

void ReferencesTool::paint(QPainter& painter, const KoViewConverter& converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}


void ReferencesTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
}

void ReferencesTool::deactivate()
{
    canvas()->canvasWidget()->setFocus();
}

QWidget* ReferencesTool::createOptionWidget()
{
    return 0;//widget;
}

#include <ReferencesTool.moc>
