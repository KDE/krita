/* This file is part of the KDE project
   Copyright 2007 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QPainter>
#include <QGridLayout>
#include <QToolButton>
#include <QLabel>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <KFileDialog>

#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>

#include "KritaShape.h"

#include "KritaShapeTool.h"
#include "KritaShapeTool.moc"

KritaShapeTool::KritaShapeTool( KoCanvasBase* canvas )
    : KoTool( canvas ),
      m_kritaShapeshape(0)
{
}

KritaShapeTool::~KritaShapeTool()
{
}

void KritaShapeTool::activate (bool temporary)
{
    Q_UNUSED( temporary );

    KoSelection* selection = m_canvas->shapeManager()->selection();
    foreach ( KoShape* shape, selection->selectedShapes() )
    {
        m_kritaShapeshape = dynamic_cast<KritaShape*>( shape );
        if ( m_kritaShapeshape )
            break;
    }
    if ( !m_kritaShapeshape )
    {
        emit sigDone();
        return;
    }
    useCursor( Qt::ArrowCursor, true );
}

void KritaShapeTool::deactivate()
{
    m_kritaShapeshape = 0;
}

void KritaShapeTool::paint( QPainter& painter, KoViewConverter& viewConverter )
{
    Q_UNUSED( painter );
    Q_UNUSED( viewConverter );
}

void KritaShapeTool::mousePressEvent( KoPointerEvent* )
{
}

void KritaShapeTool::mouseMoveEvent( KoPointerEvent* )
{
}

void KritaShapeTool::mouseReleaseEvent( KoPointerEvent* )
{
}


QWidget * KritaShapeTool::createOptionWidget()
{
    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout( optionWidget );

    QToolButton *button = 0;

    QLabel * lbl = new QLabel( i18n( "Import image" ), optionWidget );
    layout->addWidget( lbl, 0, 0 );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("open") );
    button->setToolTip( i18n( "Open" ) );
    layout->addWidget( button, 0, 1 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( slotChangeUrl() ) );

    return optionWidget;

}

void KritaShapeTool::slotChangeUrl()
{
    KUrl url = KFileDialog::getOpenUrl();
    if(!url.isEmpty() && m_kritaShapeshape)
        m_kritaShapeshape->importImage(url);
}

