/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "TextElement.h"
#include "AttributeManager.h"
#include <QFontMetricsF>
#include <KoXmlWriter.h>
#include <kdebug.h>

TextElement::TextElement( BasicElement* parent ) : TokenElement( parent )
{}

QRectF TextElement::renderToPath( const QString& raw, QPainterPath& path ) const
{
    AttributeManager manager;

    QFont font = manager.font(this);
    path.addText( path.currentPosition(), font, raw );
    QFontMetricsF fm(font);
    QRectF box = fm.boundingRect(QRect(), Qt::TextIncludeTrailingSpaces, raw).adjusted(0,-fm.ascent(),0,-fm.ascent());
    return box;
}

ElementType TextElement::elementType() const
{
    return Text;
}
