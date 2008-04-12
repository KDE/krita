/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoTextOdfSaveHelper.h"

#include <KoXmlWriter.h>
#include <KoOdf.h>
#include "KoTextShapeData.h"

struct KoTextOdfSaveHelper::Private
{
    Private( KoTextShapeData * shapeData, int from, int to )
    : shapeData( shapeData )
    , from( from )
    , to( to )
    {}

    KoTextShapeData * shapeData;
    int from;
    int to;
};


KoTextOdfSaveHelper::KoTextOdfSaveHelper( KoTextShapeData * shapeData, int from, int to )
: d( new Private( shapeData, from, to ) )
{
}

KoTextOdfSaveHelper::~KoTextOdfSaveHelper()
{
    delete d;
}

bool KoTextOdfSaveHelper::writeBody()
{
    if ( d->to < d->from )
        qSwap( d->to, d->from );

    KoXmlWriter & bodyWriter = m_context->xmlWriter();
    bodyWriter.startElement( "office:body" );
    bodyWriter.startElement( KoOdf::bodyContentElement( KoOdf::Text, true ) );

    d->shapeData->saveOdf( *m_context, d->from, d->to );

    bodyWriter.endElement(); // office:element
    bodyWriter.endElement(); // office:body
    return true;
}
