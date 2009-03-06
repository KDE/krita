/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2006 Peter Simonsson <psn@linux.se>
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <t.zachmann@zagge.de>

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

#include "KoShapeEndLinesDocker.h"

#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasResourceProvider.h>
#include <KoDockFactory.h>
#include <KoUnitDoubleSpinBox.h>
#include <KoShapeManager.h>
#include <KoShapeBorderCommand.h>
#include <KoShapeBorderModel.h>
#include <KoSelection.h>
#include "../shapecollection/KoCollectionItemModel.h"
#include <KoShapeFactory.h>
#include <KoShapeRegistry.h>
#include <klocale.h>
#include <KoOdfReadStore.h>
#include <kstandarddirs.h>
#include <KoToolManager.h>
#include <KoShape.h> 
#include <KoSelection.h>
#include <KoPathShape.h>
#include <KoLineEnd.h>


#include <QLabel>
#include <QRadioButton>
#include <QWidget>
#include <QGridLayout>
#include <QButtonGroup>
#include <QGridLayout>
#include <QList>
#include <QComboBox>
#include <QSvgRenderer>
#include <QPixmap>
#include <QMetaType>


KoShapeEndLinesDocker::KoShapeEndLinesDocker()
{
    setWindowTitle( i18n( "End Lines" ) );

    QWidget *mainWidget = new QWidget( this );
    QGridLayout *mainLayout = new QGridLayout( mainWidget );

    // Define the two QComboBox
    m_iconW = 20;
    m_iconH = 20;
    m_iconSize.setHeight(m_iconH);
    m_iconSize.setWidth(m_iconW);
    m_beginEndLineComboBox = new QComboBox( mainWidget );
    m_beginEndLineComboBox->setIconSize(m_iconSize);
    
    qRegisterMetaType<KoLineEnd>("KoLineEnd");

    QPixmap qp(m_iconSize);
    qp.fill(Qt::transparent);
    QVariant beginNone;
    QVariant endNone;
    m_beginEndLineComboBox->addItem(QIcon(qp),"None", beginNone.fromValue( KoLineEnd() ) );
    m_endEndLineComboBox = new QComboBox( mainWidget );
    m_endEndLineComboBox->setIconSize(m_iconSize);
    m_endEndLineComboBox->addItem(QIcon(qp),"None", beginNone.fromValue( KoLineEnd() ) );

    int proportion = 3;
    QString fileName( KStandardDirs::locate( "data","kpresenter/endLineStyle/endLine.xml" ) );
    if ( ! fileName.isEmpty() ) {
        QFile file( fileName );
        QString errorMessage;
        if ( KoOdfReadStore::loadAndParse( &file, m_doc, errorMessage, fileName ) ) {
            KoXmlElement drawMarker, lineEndElement(m_doc.namedItem("lineends").toElement());

            forEachElement(drawMarker, lineEndElement){

                KoLineEnd temp(drawMarker.attribute("display-name"),drawMarker.attribute("d"),drawMarker.attribute("viewBox"));

                QIcon drawIcon(temp.drawIcon(m_iconSize, proportion));
                
                QVariant beginValue;
                QVariant endValue;
                // add icon and label in the two QComboBox
                m_beginEndLineComboBox->addItem( drawIcon, temp.name(), beginValue.fromValue( temp ) );
                m_endEndLineComboBox->addItem( drawIcon, temp.name(), endValue.fromValue( temp ) );
            }
        }else {
            kDebug() << "reading of endLine.xml failed:" << errorMessage;
        }
    }
    else {
        kDebug() << "endLine.xml not found";
    }

    // Add Begin to the docker
    QLabel * beginEndLineLabel = new QLabel( i18n( "Begin :" ), mainWidget );
    mainLayout->addWidget( beginEndLineLabel, 0, 0 );
    mainLayout->addWidget( m_beginEndLineComboBox,0,1,1,3);
    connect( m_beginEndLineComboBox, SIGNAL(activated( int ) ), this, SLOT( beginEndLineChanged( int ) ) );

    // Add End to the docker
    QLabel * endEndLineLabel = new QLabel( i18n( "End :" ), mainWidget );
    mainLayout->addWidget( endEndLineLabel, 1, 0 );
    mainLayout->addWidget( m_endEndLineComboBox,1,1,1,3);
    connect( m_endEndLineComboBox, SIGNAL(activated( int ) ), this, SLOT( endEndLineChanged( int ) ) );


    mainLayout->setRowStretch( 5, 1 );
    mainLayout->setColumnStretch( 1, 1 );
    mainLayout->setColumnStretch( 2, 1 );
    mainLayout->setColumnStretch( 3, 1 );

    setWidget( mainWidget );

}

KoShapeEndLinesDocker::~KoShapeEndLinesDocker()
{
}

void KoShapeEndLinesDocker::applyChanges()
{
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    if( ! selection || ! selection->count() )
        return;
   
    KoLineEnd begin ( m_endEndLineComboBox->itemData( m_endEndLineComboBox->currentIndex() ).value<KoLineEnd>() );
    KoLineEnd end( m_endEndLineComboBox->itemData( m_endEndLineComboBox->currentIndex() ).value<KoLineEnd>() ) ;

    KoShape* shape = selection->firstSelectedShape();
    KoPathShape *pathShape;
    if(pathShape = dynamic_cast<KoPathShape*>(shape)){
        pathShape->setBeginLineEnd(begin);
        pathShape->setEndLineEnd(end);
    }
}


void KoShapeEndLinesDocker::beginEndLineChanged(int index)
{
    m_beginEndLineComboBox->setCurrentItem(index);
    m_beginEndLineCurrentName = m_beginEndLineComboBox->itemText(index);
    applyChanges();
}

void KoShapeEndLinesDocker::endEndLineChanged(int index)
{
    m_endEndLineComboBox->setCurrentItem(index);
    m_endEndLineCurrentName = m_endEndLineComboBox->itemText(index);
    applyChanges();
}

void KoShapeEndLinesDocker::selectionChanged()
{
}

void KoShapeEndLinesDocker::setCanvas( KoCanvasBase *canvas )
{
}
#include "KoShapeEndLinesDocker.moc"

