/*
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_small_preset_handler.h"

#include <KoResourceModel.h>
#include <QAbstractScrollArea>

#include "kis_paintop_registry.h"

WdgSmallPresetHandler::WdgSmallPresetHandler(QWidget* parent)
                      : QWidget(parent)
{
    setupUi(this);
    this->smallPresetChooser->showButtons(false);
    //this->smallPresetChooser->setViewMode(KisPresetChooser::THUMBNAIL);
    this->smallPresetChooser->setViewMode(KisPresetChooser::STRIP);
    antiOOPHack = this->smallPresetChooser->findChild<KoResourceItemView*>();
    antiOOPHack->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    antiOOPHack->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    //smallPresetChooser->setShowAll(false);
    
    qDebug() << "AERSH";
/*
    connect(parent, SIGNAL(paintopActivated(QString)),
            smallPresetChooser, SLOT(searchTextChanged(QString)));
            */
    connect(this->smallPresetChooser, SIGNAL(resourceSelected(KoResource*)),
            parent, SLOT(resourceSelected(KoResource*)));
}

void WdgSmallPresetHandler::currentPaintopChanged(QString printme)
{
    qDebug() << printme;
    
//    KisPaintOpPresetSP preset = m_canvas->resourceManager()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
    //if (preset) {
        //m_presetChooser->setPresetFilter(preset->paintOp());
    //}
    
    KoID paintOp;
    KoID test("", "");
    foreach(paintOp, KisPaintOpRegistry::instance()->listKeys() ) {
        if (paintOp.id() == printme) {
            qDebug() << "NOTAN eureka: " << paintOp;
            test = paintOp;
            break;
        }
    }
    
    if (test.id() != "")
        smallPresetChooser->setPresetFilter(test);
    else
        qDebug() << "no hay ID";
    
    
}

void WdgSmallPresetHandler::on_leftScrollBtn_pressed()
{
    //QPoint newcoor = scrollCoordinate(-40);
    //QPoint newcoor(-3 + antiOOPHack->width() / 2, 0);
    QPoint newcoor(-10, 0);
    antiOOPHack->scrollTo(antiOOPHack->indexAt(newcoor), QAbstractItemView::EnsureVisible);
}

void WdgSmallPresetHandler::on_rightScrollBtn_pressed()
{
    //QPoint newcoor = scrollCoordinate(+40);
    //QPoint newcoor(3 + antiOOPHack->width() / 2, 0);
    QPoint newcoor(3 + antiOOPHack->viewport()->width(), 0);
    antiOOPHack->scrollTo(antiOOPHack->indexAt(newcoor), QAbstractItemView::EnsureVisible);
}
/*
QPoint WdgSmallPresetHandler::scrollCoordinate(qint8 dx)
{
    m_coorX = dx + antiOOPHack->width() / 2;

    int leftcap = 0;
    int rightcap = antiOOPHack->columnWidth(0) * antiOOPHack->model()->columnCount() - 0;
    
    m_coorX = qBound<int>(leftcap, m_coorX, rightcap);
    qDebug() << "m_coorX: " << m_coorX;

    // antiOOPHack->columnWidth(0) * antiOOPHack->model()->columnCount();    REAL WIDTH of the model
    return QPoint(m_coorX, 0);
}
*/

#include "kis_small_preset_handler.moc"