/* This file is part of the KDE project
 * Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>
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

#include "KoPABackgroundToolFactory.h"

//KOffice includes
#include <KoPACanvas.h>

#include "KoPABackgroundTool.h"

KoPABackgroundToolFactory::KoPABackgroundToolFactory()
    : KoToolFactoryBase("KoPABackgroundTool")
{
    setToolTip( i18n( "Background editing tool" ) );
    setToolType( mainToolType() );
    setActivationShapeId("flake/always");
    setIcon( "backgroundtool" );
    setPriority( 3 );
}

KoPABackgroundToolFactory::~KoPABackgroundToolFactory()
{
}

KoToolBase * KoPABackgroundToolFactory::createTool(KoCanvasBase *canvas)
{
    return new KoPABackgroundTool( canvas );
}

bool KoPABackgroundToolFactory::canCreateTool( KoCanvasBase *canvas ) const
{
    KoPACanvas *paCanvas = dynamic_cast<KoPACanvas *>(canvas);
    return paCanvas != 0; // we only work in KoPACanvas
}
