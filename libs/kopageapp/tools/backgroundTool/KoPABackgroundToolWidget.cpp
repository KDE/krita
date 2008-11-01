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

#include "KoPABackgroundToolWidget.h"

#include <kurl.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>

#include <KoImageCollection.h>
#include <KoPatternBackground.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoShapeBackgroundCommand.h>

#include "KoPABackgroundTool.h"
#include "KoPageApp.h"

KoPABackgroundToolWidget::KoPABackgroundToolWidget( KoPABackgroundTool *tool, QWidget *parent )
: QWidget( parent )
, m_tool( tool )
{
    widget.setupUi( this );
    connect( widget.backgroundImage, SIGNAL( clicked( bool ) ), this, SLOT( setBackgroundImage() ) );
}

KoPABackgroundToolWidget::~KoPABackgroundToolWidget()
{
}

void KoPABackgroundToolWidget::setBackgroundImage()
{
    // TODO only make images selectable
    KoImageCollection * collection = dynamic_cast<KoImageCollection *>( m_tool->canvas()->shapeController()->dataCenter( "ImageCollection" ) );
    Q_ASSERT( collection );
    KoShape * page = m_tool->canvas()->resourceProvider()->koShapeResource( KoPageApp::CurrentPage );
    Q_ASSERT( page );
    if ( !collection || !page ) {
        return;
    }

    KUrl url = KFileDialog::getOpenUrl();
    if ( !url.isEmpty() ) {
        QString tmpFile;
        if ( KIO::NetAccess::download(  url, tmpFile, 0 ) ) {
            QImage image( tmpFile );
            if ( !image.isNull() ) {
                QUndoCommand * cmd = new QUndoCommand( i18n( "Change backgound image" ) );
                KoPatternBackground * bg = new KoPatternBackground( collection );
                bg->setPattern( image );
                new KoShapeBackgroundCommand( page, bg, cmd );
                m_tool->canvas()->addCommand( cmd );
            }
        }

    }
}

#include "KoPABackgroundToolWidget.moc"
