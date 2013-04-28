/* This file is part of the Calligra project, made within the KDE community.
 *
 * Copyright (C) 2013 Friedrich W. H. Kossebau <friedrich@kogmbh.com>
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

#include "TextDocumentInspectionDockerFactory.h"
#include "TextDocumentInspectionDocker.h"


TextDocumentInspectionDockerFactory::TextDocumentInspectionDockerFactory()
{
}

QString TextDocumentInspectionDockerFactory::id() const
{
    return QLatin1String("TextDocumentInspectionDocker");
}

KoDockFactoryBase::DockPosition TextDocumentInspectionDockerFactory::defaultDockPosition() const
{
    return DockRight;
}

QDockWidget* TextDocumentInspectionDockerFactory::createDockWidget()
{
    TextDocumentInspectionDocker * widget = new TextDocumentInspectionDocker();
    widget->setObjectName(id());

    return widget;
}

bool TextDocumentInspectionDockerFactory::isCollapsable() const
{
    return false;
}
