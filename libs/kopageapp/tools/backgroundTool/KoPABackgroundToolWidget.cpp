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
#include <KoDocumentResourceManager.h>

#include "KoPABackgroundTool.h"
#include <KoPageApp.h>
#include <KoPAPage.h>
#include <KoPAView.h>
#include <KoPADocument.h>
#include "commands/KoPADisplayMasterBackgroundCommand.h"
#include "commands/KoPADisplayMasterShapesCommand.h"

KoPABackgroundToolWidget::KoPABackgroundToolWidget( KoPABackgroundTool *tool, QWidget *parent )
: QWidget( parent )
, m_tool( tool )
{
    setObjectName( "KoPABackgroundToolWidget" );
    widget.setupUi( this );

    // adapt texts to type of pages
    const bool isSlideType = (m_tool->view()->kopaDocument()->pageType() == KoPageApp::Slide);
    const QString useMasterBackgroundText =
        isSlideType ? i18n("Use background of master slide") : i18n("Use background of master page");
    widget.useMasterBackground->setText(useMasterBackgroundText);
    const QString displayMasterShapesText =
        isSlideType ? i18n("Display shapes of master slide") : i18n("Display shapes of master page");
    widget.displayMasterShapes->setText(displayMasterShapesText);

    connect(widget.useMasterBackground, SIGNAL(clicked(bool)), SLOT(useMasterBackground(bool)));
    connect(widget.backgroundImage, SIGNAL(clicked(bool)), SLOT(setBackgroundImage()));
    connect(widget.displayMasterShapes, SIGNAL(clicked(bool)), SLOT(displayMasterShapes(bool)));

    connect(m_tool->view()->proxyObject, SIGNAL(activePageChanged()), SLOT(slotActivePageChanged()));

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

        const bool enableBackgroundEditing = (! normalPage->displayMasterBackground());
        widget.backgroundImage->setEnabled(enableBackgroundEditing);
        widget.useStrokeAndFillDockerLabel->setEnabled(enableBackgroundEditing);
    }
    else {
        widget.useMasterBackground->setChecked( false );
        widget.displayMasterShapes->setChecked( false );

        widget.backgroundImage->setEnabled(true);
        widget.useStrokeAndFillDockerLabel->setEnabled(true);
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
                const bool isSlideType = (m_tool->view()->kopaDocument()->pageType() == KoPageApp::Slide);
                const QString commandTitle = isSlideType ?
                    i18nc( "(qtundo-format)", "Change slide background image") :
                    i18nc( "(qtundo-format)", "Change page background image");
                KUndo2Command * cmd = new KUndo2Command(commandTitle);
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

void KoPABackgroundToolWidget::useMasterBackground(bool doUse)
{
    KoPAPage * page = dynamic_cast<KoPAPage *>( m_tool->canvas()->resourceManager()->koShapeResource( KoPageApp::CurrentPage ) );
    if ( page ) {
        KoPADisplayMasterBackgroundCommand * cmd = new KoPADisplayMasterBackgroundCommand(page, doUse);
        m_tool->canvas()->addCommand( cmd );
    }

    const bool enableBackgroundEditing = (! doUse);
    widget.backgroundImage->setEnabled(enableBackgroundEditing);
    widget.useStrokeAndFillDockerLabel->setEnabled(enableBackgroundEditing);
}

void KoPABackgroundToolWidget::displayMasterShapes(bool doDisplay)
{
    KoPAPage * page = dynamic_cast<KoPAPage *>( m_tool->canvas()->resourceManager()->koShapeResource( KoPageApp::CurrentPage ) );
    if ( page ) {
        KoPADisplayMasterShapesCommand * cmd = new KoPADisplayMasterShapesCommand(page, doDisplay);
        m_tool->canvas()->addCommand( cmd );
    }
}

#include <KoPABackgroundToolWidget.moc>
