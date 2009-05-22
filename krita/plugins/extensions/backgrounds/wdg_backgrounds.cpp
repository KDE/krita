/*
 *  dlg_backgrounds.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "wdg_backgrounds.h"
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include "kis_factory2.h"

#include <QStringList>
#include <QImage>
#include <QListWidgetItem>
#include <QDebug>

WdgBackgrounds::WdgBackgrounds(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    QStringList backgroundFileNames =
            KisFactory2::componentData().dirs()->findAllResources("kis_backgrounds", "*.png");

    foreach(QString fileName, backgroundFileNames) {
        QImage img = QImage(fileName).copy(0, 0, 64, 64);
        QListWidgetItem* item = new QListWidgetItem(lstBackgrounds);
        //item->setData(Qt::DisplayRole, fileName);
        item->setData(Qt::DecorationRole, img);
    }

    bnAdd->setVisible( false );
    bnAdd->setIcon( SmallIcon( "list-add" ) );
    bnAdd->setToolTip( i18n("Import existing background") );
    bnAdd->setEnabled( true );

    bnRemove->setVisible( false );
    bnRemove->setIcon( SmallIcon( "list-remove" ) );
    bnRemove->setToolTip( i18n("Remove currently selected background") );
    bnRemove->setEnabled( true );

    bnReset->setVisible( false );
    bnReset->setIcon( SmallIcon( "edit-undo" ) );
    bnReset->setToolTip( i18n("Reset to default") );
    bnReset->setEnabled( true );

}

void WdgBackgrounds::addClicked()
{
}

void WdgBackgrounds::deleteClicked()
{
}

void WdgBackgrounds::resetClicked()
{
}

#include "wdg_backgrounds.moc"
