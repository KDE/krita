/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>

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

#include <QImage>
#include <QModelIndex>
#include <QTextDocument>
#include <QUrl>
#include <klocale.h>
#include "KoDocumentSectionModel.h"
#include "KoDocumentSectionToolTip.h"

KoDocumentSectionToolTip::KoDocumentSectionToolTip()
{
}

KoDocumentSectionToolTip::~KoDocumentSectionToolTip()
{
}

QTextDocument *KoDocumentSectionToolTip::createDocument( const QModelIndex &index )
{
    QTextDocument *doc = new QTextDocument( this );

    QImage thumb = index.data( int( Model::BeginThumbnailRole ) + 250 ).value<QImage>();
    doc->addResource( QTextDocument::ImageResource, QUrl( "data:thumbnail" ), thumb );

    QString name = index.data( Qt::DisplayRole ).toString();
    Model::PropertyList properties = index.data( Model::PropertiesRole ).value<Model::PropertyList>();
    QString rows;
    for( int i = 0, n = properties.count(); i < n; ++i )
    {
        const QString row = QString( "<tr><td align=\"right\">%1</td><td align=\"left\">%2</td></tr>" );
        const QString value = properties[i].isMutable
                      ? ( properties[i].state.toBool() ? i18n( "Yes" ) : i18n( "No" ) )
                      : properties[i].state.toString();
        rows.append( row.arg( i18n( "%1:", properties[i].name ) ).arg( value ) );
    }

    rows = QString( "<table>%1</table>" ).arg( rows );

    const QString image = QString( "<table border=\"1\"><tr><td><img src=\"data:thumbnail\"></td></tr></table>" );
    const QString body = QString( "<h3 align=\"center\">%1</h3>" ).arg( name )
                       + QString( "<table><tr><td>%1</td><td>%2</td></tr></table>" ).arg( image ).arg( rows );
    const QString html = QString( "<html><body>%1</body></html>" ).arg( body );

    doc->setHtml( html );
    doc->setTextWidth( qMin( doc->size().width(), 500.0 ) );

    return doc;
}

#include "KoDocumentSectionToolTip.moc"
