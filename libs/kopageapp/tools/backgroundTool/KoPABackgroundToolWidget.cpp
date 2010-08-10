/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Thorsten Zachmann <zachmann@kde.org>
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
#include "KoPAPage.h"
#include "KoPAView.h"
#include "commands/KoPADisplayMasterBackgroundCommand.h"
#include "commands/KoPADisplayMasterShapesCommand.h"

KoPABackgroundToolWidget::KoPABackgroundToolWidget( KoPABackgroundTool *tool, QWidget *parent )
: QWidget( parent )
, m_tool( tool )
{
    setObjectName( "KoPABackgroundToolWidget" );
    widget.setupUi( this );
    connect( widget.backgroundImage, SIGNAL( clicked( bool ) ), this, SLOT( setBackgroundImage() ) );
    connect( widget.useMasterBackground, SIGNAL( stateChanged( int ) ), this, SLOT( useMasterBackground( int ) ) );
    connect( widget.displayMasterShapes, SIGNAL( stateChanged( int ) ), this, SLOT( displayMasterShapes( int ) ) );

    connect( m_tool->view()->proxyObject, SIGNAL( activePageChanged() ), this, SLOT( slotActivePageChanged() ) );

    slotActivePageChanged();
}

KoPABackgroundToolWidget::~KoPABackgroundToolWidget()
{
}

void KoPABackgroundToolWidget::slotActivePageChanged()
{
    KoPAPageBase * page = m_tool->view()->activePage();

    KoPAPage * normalPage = dynamic_cast<KoPAPage *>( page );

    widget.useMasterBackground->setEnabled( normalPage );
    widget.displayMasterShapes->setEnabled( normalPage );
    if ( normalPage ) {
        widget.useMasterBackground->setChecked( normalPage->displayMasterBackground() );
        widget.displayMasterShapes->setChecked( normalPage->displayMasterShapes() );
    }
    else {
        widget.useMasterBackground->setChecked( false );
        widget.displayMasterShapes->setChecked( false );
    }
}

void KoPABackgroundToolWidget::setBackgroundImage()
{
    // TODO only make images selectable
    KoImageCollection *collection = m_tool->canvas()->shapeController()->resourceManager()->imageCollection();
    Q_ASSERT( collection );
    KoShape * page = m_tool->canvas()->resourceManager()->koShapeResource( KoPageApp::CurrentPage );
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
                QUndoCommand * cmd = new QUndoCommand( i18n( "Change background image" ) );
                KoPatternBackground * bg = new KoPatternBackground( collection );
                bg->setPattern( image );
                QSizeF imageSize = bg->patternOriginalSize();
                QSizeF pageSize = m_tool->view()->activePage()->size();
                KoPatternBackground::PatternRepeat repeat = KoPatternBackground::Original;
                if ( imageSize.width() > pageSize.width() || imageSize.height() > pageSize.height() ) {
                    qreal imageRatio = imageSize.width() / imageSize.height();
                    qreal pageRatio = pageSize.width() / pageSize.height();
                    if ( qAbs( imageRatio - pageRatio) < 0.1 ) {
                        repeat = KoPatternBackground::Stretched;
                    }
                    else {
                        qreal zoom = pageSize.width() / imageSize.width();
                        zoom = qMin( zoom, pageSize.height() / imageSize.height() );

                        bg->setPatternDisplaySize( imageSize * zoom );
                    }
                }
                bg->setRepeat( repeat );

                new KoShapeBackgroundCommand( page, bg, cmd );
                m_tool->canvas()->addCommand( cmd );
            }
        }

    }
}

void KoPABackgroundToolWidget::useMasterBackground( int state )
{
    KoPAPage * page = dynamic_cast<KoPAPage *>( m_tool->canvas()->resourceManager()->koShapeResource( KoPageApp::CurrentPage ) );
    if ( page ) {
        KoPADisplayMasterBackgroundCommand * cmd = new KoPADisplayMasterBackgroundCommand( page, state == Qt::Checked );
        m_tool->canvas()->addCommand( cmd );
    }
}

void KoPABackgroundToolWidget::displayMasterShapes( int state )
{
    KoPAPage * page = dynamic_cast<KoPAPage *>( m_tool->canvas()->resourceManager()->koShapeResource( KoPageApp::CurrentPage ) );
    if ( page ) {
        KoPADisplayMasterShapesCommand * cmd = new KoPADisplayMasterShapesCommand( page, state == Qt::Checked );
        m_tool->canvas()->addCommand( cmd );
    }
}

#include <KoPABackgroundToolWidget.moc>
