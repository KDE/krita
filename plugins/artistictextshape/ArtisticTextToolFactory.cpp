/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "ArtisticTextToolFactory.h"
#include "ArtisticTextTool.h"
#include "ArtisticTextShape.h"

#include <klocale.h>

ArtisticTextToolFactory::ArtisticTextToolFactory()
    : KoToolFactoryBase("ArtisticTextToolFactoryID")
{
    setToolTip( i18n("Artistic Text Editing Tool") );
    setToolType( dynamicToolType() );
    setIcon ("artistictext-tool");
    setPriority( 1 );
    setActivationShapeId( ArtisticTextShapeID );
}

ArtisticTextToolFactory::~ArtisticTextToolFactory()
{
}

KoToolBase * ArtisticTextToolFactory::createTool( KoCanvasBase * canvas )
{
    return new ArtisticTextTool( canvas );
}

#include <ArtisticTextToolFactory.moc>
