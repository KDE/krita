/* This file is part of the KDE project
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "ArtisticTextShapeLoadingUpdater.h"

#include "ArtisticTextShape.h"
#include <KoPathShape.h>

ArtisticTextShapeLoadingUpdater::ArtisticTextShapeLoadingUpdater(ArtisticTextShape * artisticTextShape)
: m_artisticTextShape(artisticTextShape)
{
}

ArtisticTextShapeLoadingUpdater::~ArtisticTextShapeLoadingUpdater()
{
}

void ArtisticTextShapeLoadingUpdater::update(KoShape * shape)
{
    // we have already loaded the correct transformation, so save it here
    // and apply after putting us on the path shape
    QTransform matrix = m_artisticTextShape->transformation();
    
    // putting us on the path shape resulting in a changed transformation
    m_artisticTextShape->putOnPath(dynamic_cast<KoPathShape*>(shape));

    // resetting the transformation to the former state
    m_artisticTextShape->setTransformation( matrix );
}
