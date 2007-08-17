/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "PathToolOptionWidget.h"
#include "KoPathTool.h"
#include "KoPathShape.h"
#include "KoShapeFactory.h"
#include "KoShapeRegistry.h"
#include "KoShapeConfigWidgetBase.h"
#include "KoToolManager.h"
#include "KoCanvasController.h"
#include "KoCanvasBase.h"

PathToolOptionWidget::PathToolOptionWidget(KoPathTool *tool, QWidget *parent)
    : QWidget(parent), m_tool(tool), m_path(0), m_configPanel(0)
{
    widget.setupUi(this);
    widget.corner->setDefaultAction(tool->action("pathpoint-corner"));
    widget.smooth->setDefaultAction(tool->action("pathpoint-smooth"));
    widget.symmetric->setDefaultAction(tool->action("pathpoint-symmetric"));
    widget.lineSegment->setDefaultAction(tool->action("pathsegment-line"));
    widget.curveSegment->setDefaultAction(tool->action("pathsegment-curve"));
    widget.addPoint->setDefaultAction(tool->action("pathpoint-insert"));
    widget.removePoint->setDefaultAction(tool->action("pathpoint-remove"));
    widget.breakPoint->setDefaultAction(tool->action("path-break-point"));
    widget.breakSegment->setDefaultAction(tool->action("path-break-segment"));
    widget.joinSegment->setDefaultAction(tool->action("pathpoint-join"));
    widget.mergePoints->setDefaultAction(tool->action("pathpoint-merge"));

    connect(widget.convertToPath, SIGNAL(released()), tool->action("convert-to-path"), SLOT(trigger()));
}

PathToolOptionWidget::~PathToolOptionWidget() {
}

void PathToolOptionWidget::setSelectionType(int type) {
    const bool plain = type & PlainPath;
    if( plain )
        widget.stackedWidget->setCurrentIndex(0);
    else
        widget.stackedWidget->setCurrentIndex(1);
}

void PathToolOptionWidget::setSelectedPath( KoPathShape * path )
{
    // remove the config widget if a null path is set, or the path has changed
    if( ! path || path != m_path )
    {
        while( widget.configWidget->count() )
            widget.configWidget->removeWidget( widget.configWidget->widget( 0 ) );
    }

    if( ! path )
    {
        m_path = 0;
        m_configPanel = 0;
        return;
    }
    else if( path != m_path )
    {
        // when a path is set and is differs from the previous one
        // get the config widget and insert it into the option widget
        m_path = path;
        if( ! m_path )
            return;
        KoShapeFactory *factory = KoShapeRegistry::instance()->value( m_path->pathShapeId() );
        if( ! factory )
            return;
        QList<KoShapeConfigWidgetBase*> panels = factory->createShapeOptionPanels();
        if( ! panels.count() )
            return;

        m_configPanel = panels.first();
        widget.configWidget->insertWidget( 0, m_configPanel );
        connect( m_configPanel, SIGNAL(propertyChanged()), this, SLOT(shapePropertyChanged()));
    }

    if( m_configPanel )
        m_configPanel->open( m_path );
}

void PathToolOptionWidget::shapePropertyChanged()
{
    if( m_configPanel )
    {
        QUndoCommand * cmd = m_configPanel->createCommand();
        if( ! cmd )
            return;
        KoToolManager::instance()->activeCanvasController()->canvas()->addCommand( cmd );
    }
}

#include <PathToolOptionWidget.moc>
