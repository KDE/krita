/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>
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

// Own
#include "VectorTool.h"

// Qt
#include <QAction>
#include <QGridLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QPainter>

// KDE
#include <KLocale>
#include <KIcon>

// Koffice
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>


VectorTool::VectorTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_currentShape(0)
{
}

VectorTool::~VectorTool()
{
}

void VectorTool::paint( QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    // nothing to do here
}

void VectorTool::mousePressEvent( KoPointerEvent *event )
{
    event->ignore();
}

void VectorTool::mouseMoveEvent( KoPointerEvent *event )
{
    event->ignore();
}

void VectorTool::mouseReleaseEvent( KoPointerEvent *event )
{
    event->ignore();
}

void VectorTool::activate (bool)
{
    KoSelection *selection = canvas()->shapeManager()->selection();
    foreach (KoShape *shape, selection->selectedShapes()) {
        m_currentShape = dynamic_cast<VectorShape*> (shape);
        if (m_currentShape)
            break;
    }
    if (m_currentShape == 0) { // none found
        emit done();
        return;
    }

    // updateActions();
}

void VectorTool::deactivate()
{
    m_currentShape = 0;
}

void VectorTool::updateActions()
{
}

void VectorTool::setPrintable(bool on)
{
    if (m_currentShape)
        m_currentShape->setPrintable(on);
}

QWidget *VectorTool::createOptionWidget()
{
    QWidget *widget = new QWidget();

    return widget;
}

#include <VectorTool.moc>
