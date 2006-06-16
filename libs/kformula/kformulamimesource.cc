/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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
 * Boston, MA 02110-1301, USA.
*/

#include <iostream>

#include <q3popupmenu.h>
#include <QBuffer>
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPixmap>

#include <QList>
#include <QImageWriter>
#include <kcommand.h>

#include "contextstyle.h"
#include "formulacursor.h"
#include "formulaelement.h"
#include "kformuladocument.h"
#include "kformulamimesource.h"

KFORMULA_NAMESPACE_BEGIN
using namespace std;


MimeSource::MimeSource(Document* doc, const QDomDocument& formula)
        : formulaDocument( doc ), document(formula)
{
    // The query for text/plain comes very often. So make sure
    // it's fast.

    rootElement = new FormulaElement(this);
    FormulaCursor cursor(rootElement);

    QList<BasicElement*> list;
//    list.setAutoDelete(true);
    if ( cursor.buildElementsFromDom( document.documentElement(), list ) ) {
        cursor.insert(list);
//        latexString = rootElement->toLatex().toUtf8();
        if (latexString.size() > 0) {
            latexString.truncate(latexString.size()-1);
        }
    }
}

MimeSource::~MimeSource()
{
    delete rootElement;
}

const char * MimeSource::selectionMimeType()
{
    return "application/x-kformula";
}

const char* MimeSource::format( int n ) const
{
    switch (n) {
        case 0:
            return selectionMimeType();
        case 1:
            return "image/ppm";
        case 2:
            return "text/plain";
        case 3:
            return "text/x-tex";
    }
    return NULL;
}

bool MimeSource::provides( const char * format) const
{
//This is not completed
    if(QString(format)==selectionMimeType())
        return true;
    else if(QString(format)=="image/ppm")
        return true;
    else if(QString(format)=="text/plain")
        return true;
    else if(QString(format)=="text/x-tex")
        return true;
    else
        return false;
}

QByteArray MimeSource::encodedData ( const char *format ) const
{
    QString fmt=format;  //case sensitive?

    if ((fmt=="text/plain") || (fmt=="text/x-tex"))
        return latexString;

    if (fmt==selectionMimeType()) {
	QByteArray d=document.toByteArray();
  	d.truncate(d.size()-1);
	return d;
    }

    if (fmt=="image/ppm") {

	//cerr << "asking image" << endl;
        ContextStyle& context = formulaDocument->getContextStyle( false );
        //context.setResolution(5, 5);

        rootElement->calcSizes(context);
        QRectF rect(rootElement->getX(), rootElement->getY(),
                   rootElement->getWidth(), rootElement->getHeight());

    	QPixmap pm( context.layoutUnitToPixelX( rootElement->getWidth() ),
                    context.layoutUnitToPixelY( rootElement->getHeight() ) );
	pm.fill();
	QPainter paint(&pm);
        rootElement->draw(paint, rect, context);
	paint.end();

	QByteArray d;
	QBuffer buff(&d);
	buff.open(QIODevice::WriteOnly);
	QImageWriter io(&buff,"PPM");
	QImage ima=pm.toImage();
	ima.detach();
	//io.write(ima);
	if(!io.write(ima))
	    return QByteArray();

	buff.close();
    	return d;
    }

    return QByteArray();
}

const SymbolTable& MimeSource::getSymbolTable() const
{
    return formulaDocument->getContextStyle( false ).symbolTable();
}

KFORMULA_NAMESPACE_END
