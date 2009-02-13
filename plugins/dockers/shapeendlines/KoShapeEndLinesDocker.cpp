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


class KoShapeEndLinesDocker::Private
{
public:
    Private() {}
    QComboBox * beginEndLineComboBox;
    QComboBox * endEndLineComboBox;
};

QByteArray KoShapeEndLinesDocker::generateSVG(QString path, QString viewBox, QSize size, QString comment)
{
    QByteArray str("<?xml version=\"1.0\" standalone=\"no\"?>\
    <!--");
    str.append(comment.toUtf8());
    str.append("-->\
    <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\
    <svg width=\"");
    str.append(QString::number(size.width()).toUtf8());
    str.append("px\" height=\"");
    str.append(QString::number(size.height()).toUtf8());
    str.append("px\" viewBox=\"");
    str.append(viewBox.toUtf8());
    str.append("\" preserveAspectRatio=\"none\" xmlns=\"http://www.w3.org/2000/svg\">\
    <path fill=\"black\"  d=\"");
    str.append(path.toUtf8());
    str.append("\" />\
    </svg>");
    return str;
}

KoShapeEndLinesDocker::KoShapeEndLinesDocker()
    : d( new Private() )
{
    int proportion = 3;
    setWindowTitle( i18n( "End Lines" ) );

    QWidget *mainWidget = new QWidget( this );
    QGridLayout *mainLayout = new QGridLayout( mainWidget );

    // Define the two QComboBox
    iconW = 20;
    iconH = 20;
    iconSize.setHeight(iconH);
    iconSize.setWidth(iconW);
    d->beginEndLineComboBox = new QComboBox( mainWidget );
    d->beginEndLineComboBox->setIconSize(iconSize);
    d->beginEndLineComboBox->addItem(QIcon(),"None");
    d->endEndLineComboBox = new QComboBox( mainWidget );
    d->endEndLineComboBox->setIconSize(iconSize);
    d->endEndLineComboBox->addItem(QIcon(),"None");

    QString fileName( KStandardDirs::locate( "data", "kpresenter/endLineStyle/endLine.xml" ) );
    if ( ! fileName.isEmpty() ) {
        QFile file( fileName );
        QString errorMessage;
        if ( KoOdfReadStore::loadAndParse( &file, m_doc, errorMessage, fileName ) ) {
            KoXmlElement drawMarker, lineEndElement(m_doc.namedItem("lineends").toElement());

            QSvgRenderer endLineRenderer;
            forEachElement(drawMarker, lineEndElement){
                // Init QPainter and QPixmap
                QPixmap endLinePixmap(d->endEndLineComboBox->iconSize());
                endLinePixmap.fill(QColor(Qt::transparent));
                QPainter endLinePainter(&endLinePixmap);

                // Convert path to SVG
                endLineRenderer.load(generateSVG(drawMarker.attribute("d"), drawMarker.attribute("viewBox"), QSize(iconW-6, iconH-6)));
                endLineRenderer.render(&endLinePainter, QRectF(proportion, proportion, iconW-(proportion*2), iconH-(proportion*2)));

                // init QIcon
                QIcon drawIcon(endLinePixmap);

                // add icon and label in the two QComboBox
                d->beginEndLineComboBox->addItem(drawIcon, drawMarker.attribute("display-name"));
                d->endEndLineComboBox->addItem(drawIcon, drawMarker.attribute("display-name"));

                nameEndLineList.append(drawMarker.attribute("display-name"));
                pathEndLineMap.insert(drawMarker.attribute("display-name"), drawMarker.attribute("d"));
                viewEndLineMap.insert(drawMarker.attribute("display-name"), drawMarker.attribute("viewBox"));
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
    mainLayout->addWidget( d->beginEndLineComboBox,0,1,1,3);
    connect( d->beginEndLineComboBox, SIGNAL(activated( int ) ), this, SLOT( beginEndLineChanged(int) ) );

    // Add End to the docker
    QLabel * endEndLineLabel = new QLabel( i18n( "End :" ), mainWidget );
    mainLayout->addWidget( endEndLineLabel, 1, 0 );
    mainLayout->addWidget( d->endEndLineComboBox,1,1,1,3);
    connect( d->endEndLineComboBox, SIGNAL(activated( int ) ), this, SLOT( endEndLineChanged(int) ) );


    mainLayout->setRowStretch( 5, 1 );
    mainLayout->setColumnStretch( 1, 1 );
    mainLayout->setColumnStretch( 2, 1 );
    mainLayout->setColumnStretch( 3, 1 );

    setWidget( mainWidget );

}

KoShapeEndLinesDocker::~KoShapeEndLinesDocker()
{
    delete d;
}

void KoShapeEndLinesDocker::applyChanges()
{
//   KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
//   KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
//   if( ! selection || ! selection->count() )
//        return;
//   
//    QSvgRenderer endLineRenderer;
//                // Init QPainter and QPixmap
//                QPixmap endLinePixmap(d->beginEndLineComboBox->iconSize());
//                endLinePixmap.fill(QColor(Qt::transparent));
//                QPainter endLinePainter(&endLinePixmap);
//                // Convert path to SVG
//                endLineRenderer.load(generateSVG(drawMarker.attribute("d"), drawMarker.attribute("viewBox")));
//                endLineRenderer.render(&endLinePainter);
//
//                // init QIcon
//                QIcon drawIcon(endLinePixmap);
//
//   
//   KoShape* shape = selection->firstSelectedShape();
//   //qreal angle = shape->rotation();
//   kDebug() << selection->count();
//   //shape->isVisible(false);
}


void KoShapeEndLinesDocker::beginEndLineChanged(int index)
{
    d->beginEndLineComboBox->setCurrentItem(index);
    beginEndLineCurrentName = d->beginEndLineComboBox->itemText(index);
    applyChanges();
}

void KoShapeEndLinesDocker::endEndLineChanged(int index)
{
    d->endEndLineComboBox->setCurrentItem(index);
    endEndLineCurrentName = d->endEndLineComboBox->itemText(index);
    applyChanges();
}

void KoShapeEndLinesDocker::selectionChanged()
{
}

void KoShapeEndLinesDocker::setCanvas( KoCanvasBase *canvas )
{
}
#include "KoShapeEndLinesDocker.moc"

