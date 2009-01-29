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
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <KoOdfReadStore.h>
#include <kstandarddirs.h>


#include <QLabel>
#include <QRadioButton>
#include <QWidget>
#include <QGridLayout>
#include <QButtonGroup>
#include <QGridLayout>
#include <QList>
#include <QComboBox>
#include <QSvgRenderer>


class KoShapeEndLinesDocker::Private
{
public:
    Private() {}
    QComboBox * beginEndLineComboBox;
    QComboBox * endEndLineComboBox;
};

QByteArray KoShapeEndLinesDocker::generateSVG(QString path, QString viewBox, QString comment){
  QString str("<?xml version=\"1.0\" standalone=\"no\"?>");
  str.append("<!--");
  str.append(comment);
  str.append("-->");
  str.append("\n");
  str.append("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">");
  str.append("\n");
  str.append("<svg width=\"30px\" height=\"30px\" viewBox=\"");
  str.append(viewBox);
  str.append("\" preserveAspectRatio=\"none\" xmlns=\"http://www.w3.org/2000/svg\">");
  str.append("\n");
  str.append("  <path fill=\"black\"  d=\"");
  str.append(path);
  str.append("\" />");
  str.append("\n");
  str.append("</svg>");
  QByteArray res;
  return res.append(str.toUtf8());
}

KoShapeEndLinesDocker::KoShapeEndLinesDocker()
    : d( new Private() )
{
    setWindowTitle( i18n( "End Lines" ) );

    QWidget *mainWidget = new QWidget( this );
    QGridLayout *mainLayout = new QGridLayout( mainWidget );

    // Define the two QComboBox
    int iconW=30, iconH=30;
    d->beginEndLineComboBox = new QComboBox( mainWidget );
    d->beginEndLineComboBox->setIconSize(QSize(iconW, iconH));
    d->endEndLineComboBox = new QComboBox( mainWidget );
    d->endEndLineComboBox->setIconSize(QSize(iconW, iconH));

    QString fileName( KStandardDirs::locate( "data", "kpresenter/endLineStyle/endLine.xml" ) );
    if ( ! fileName.isEmpty() ) {
      QFile file( fileName );
      QString errorMessage;
      if ( KoOdfReadStore::loadAndParse( &file, m_doc, errorMessage, fileName ) ) {
        KoXmlElement drawMarker, lineEndElement(m_doc.namedItem("lineends").toElement());

        forEachElement(drawMarker, lineEndElement){
          // Init QPainter and QPixmap
          QIcon drawIcon;
          QPixmap endLinePixmap(iconW, iconH);
          endLinePixmap.fill(QColor(Qt::transparent));
          QPainter *endLinePainter = new QPainter(&endLinePixmap);
          // Convert path to SVG
          QSvgRenderer *endLineRenderer;
          //kDebug() << generateSVG(drawMarker.attribute("d"), drawMarker.attribute("viewBox"), drawMarker.attribute("name").replace("_", " "));
          endLineRenderer = new QSvgRenderer(generateSVG(drawMarker.attribute("d"), drawMarker.attribute("viewBox")));
          endLineRenderer->render(endLinePainter);

          // init QIcon
          drawIcon = QIcon(endLinePixmap);

          // add icon and label in the two QComboBox
          d->beginEndLineComboBox->addItem(drawIcon, drawMarker.attribute("name").replace("_", " "));
          d->endEndLineComboBox->addItem(drawIcon, drawMarker.attribute("name").replace("_", " "));

          // erease the two pointer
          delete endLineRenderer;
          delete endLinePainter;
        }
	    }else {
        kDebug() << "reading of endLine.xml failed:" << errorMessage;
      }
    }
    else {
      kDebug() << "endLine.xml not found";
    }

    
    // Add Begin to the docker
    QLabel * beginEndLineLabel = new QLabel( i18n( "Begin:" ), mainWidget );
    mainLayout->addWidget( beginEndLineLabel, 0, 0 );
    mainLayout->addWidget( d->beginEndLineComboBox,0,1,1,3);
    connect( d->beginEndLineComboBox, SIGNAL(activated( int ) ), this, SLOT( beginEndLineChanged(int) ) );

    // Add End to the docker
    QLabel * endEndLineLabel = new QLabel( i18n( "End:" ), mainWidget );
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
}


void KoShapeEndLinesDocker::beginEndLineChanged(int index)
{
  d->beginEndLineComboBox->setCurrentItem(index);
}

void KoShapeEndLinesDocker::endEndLineChanged(int index)
{
  d->endEndLineComboBox->setCurrentItem(index);
}

void KoShapeEndLinesDocker::selectionChanged()
{
}

void KoShapeEndLinesDocker::setCanvas( KoCanvasBase *canvas )
{
}
#include "KoShapeEndLinesDocker.moc"

