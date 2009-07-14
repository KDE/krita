/* This file is part of the KDE project
   Copyright 2007 Montel Laurent <montel@kde.org>

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

#include "PictureTool.h"
#include "PictureShape.h"
#include "ChangeImageCommand.h"

#include <QToolButton>
#include <QGridLayout>
#include <KLocale>
#include <KIconLoader>
#include <KUrl>
#include <KFileDialog>

#include <KoCanvasBase.h>
#include <KoImageCollection.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>

PictureTool::PictureTool( KoCanvasBase* canvas )
    : KoTool( canvas ),
      m_pictureshape(0)
{
}

void PictureTool::activate (bool temporary)
{
    Q_UNUSED( temporary );

    KoSelection* selection = m_canvas->shapeManager()->selection();
    foreach ( KoShape* shape, selection->selectedShapes() )
    {
        m_pictureshape = dynamic_cast<PictureShape*>( shape );
        if ( m_pictureshape )
            break;
    }
    if ( !m_pictureshape )
    {
        emit done();
        return;
    }
    useCursor( Qt::ArrowCursor, true );
}

void PictureTool::deactivate()
{
  m_pictureshape = 0;
}

QWidget * PictureTool::createOptionWidget()
{

    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout( optionWidget );

    QToolButton *button = 0;

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("open") );
    button->setToolTip( i18n( "Open" ) );
    layout->addWidget( button, 0, 0 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( slotChangeUrl() ) );

    return optionWidget;

    return 0;
}

void PictureTool::slotChangeUrl()
{
    //kDebug()<<" PictureTool::slotChangeUrl";
    KUrl url = KFileDialog::getOpenUrl();
    if (!url.isEmpty() && m_pictureshape) {
        KoImageData* data = m_pictureshape->imageCollection()->getImage(url);
        if (data) {
            ChangeImageCommand *cmd = new ChangeImageCommand(m_pictureshape, data);
            m_canvas->addCommand(cmd);
        }
    }
}

void PictureTool::mouseDoubleClickEvent( KoPointerEvent *event ) {
    if(m_canvas->shapeManager()->shapeAt(event->point) != m_pictureshape) {
        event->ignore(); // allow the event to be used by another
        return;
    }

    slotChangeUrl();
/*
    repaintSelection();
    updateSelectionHandler();
*/
}

#include "PictureTool.moc"
