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

#include "KoResourceModel.h"
#include "KoResourceItemView.h"
#include "KoResourceItemChooser.h"
#include "kis_paintop_registry.h"

#include <QtGui/QAbstractScrollArea>
#include <QtGui/QMouseEvent>
#include <QtCore/QTimer>

WdgSmallPresetHandler::WdgSmallPresetHandler(QWidget* parent)
                      : QWidget(parent)
{
    setupUi(this);
    smallPresetChooser->showButtons(false);
    smallPresetChooser->setViewMode(KisPresetChooser::STRIP);
    antiOOPHack = smallPresetChooser->findChild<KoResourceItemView*>();
    antiOOPHack->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    antiOOPHack->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    refresher = new QTimer(this);
    refresher->setSingleShot(true);
    
    /* This is an heuristic to fill smallPresetChooser with only the presets
     * for the paintop that comes selected by default: Pixel Brush.
     * TODO this must be replaced by a more correct approach.
     */
    const QString PIXEL_BRUSH_ID = "paintbrush";
    smallPresetChooser->setPresetFilter(KoID(PIXEL_BRUSH_ID));
    
    const QString TRASH_ICON = "trash-empty";
    deletePresetBtn->setIcon(SmallIcon(TRASH_ICON, KIconLoader::SizeSmall));
    deletePresetBtn->setVisible(true);
    
    connect(smallPresetChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(prepareDeleteButton()));
    connect(smallPresetChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(startRefreshingTimer()));
    connect(refresher, SIGNAL(timeout()), this, SLOT(repaintDeleteButton()));
}

WdgSmallPresetHandler::~WdgSmallPresetHandler()
{
    delete refresher;
}

void WdgSmallPresetHandler::showEvent(QShowEvent* event)
{
    deletePresetBtn->hide();
    QWidget::showEvent(event);
}

void WdgSmallPresetHandler::currentPaintopChanged(QString paintOpID)
{
    foreach (KoID paintOp, KisPaintOpRegistry::instance()->listKeys()) {
        if (paintOp.id() == paintOpID) {
            smallPresetChooser->setPresetFilter(paintOp);
            break;
        }
    }
    deletePresetBtn->hide();
}

void WdgSmallPresetHandler::startRefreshingTimer()
{
    // Estimated time it takes for the ResourceView to scroll when a widget that's
    // only partially visible becomes visible
    refresher->start(450);
}

void WdgSmallPresetHandler::repaintDeleteButton()
{
    if (deletePresetBtn->isVisible()) {
        prepareDeleteButton();
    }
}

void WdgSmallPresetHandler::prepareDeleteButton()
{
    const quint8 HEURISTIC_OFFSET = 3;  // This number is just conjured out of the nether to make
                                        // things look good
    quint16 buttonWidth     = deletePresetBtn->width();
    quint16 buttonHeight    = deletePresetBtn->height();
    quint16 columnWidth     = antiOOPHack->columnWidth(0);  // All columns assumed equal in width
    quint16 currentColumn   = antiOOPHack->currentIndex().column();
    quint16 rowHeight       = antiOOPHack->rowHeight(0);    // There is only 1 row in this widget
    quint16 yPos            = rowHeight - deletePresetBtn->height() + HEURISTIC_OFFSET;
    quint16 xPos            = antiOOPHack->columnViewportPosition(currentColumn)
                              + columnWidth + HEURISTIC_OFFSET - buttonWidth;
    
    deletePresetBtn->setGeometry(xPos, yPos, buttonWidth, buttonHeight);
    deletePresetBtn->setVisible(true);
}

void WdgSmallPresetHandler::on_leftScrollBtn_pressed()
{
    // Deciding how far beyond the left margin (10 pixels) was an arbitrary decision
    QPoint beyondLeftMargin(-10, 0);
    antiOOPHack->scrollTo(antiOOPHack->indexAt(beyondLeftMargin), QAbstractItemView::EnsureVisible);
    
    deletePresetBtn->setVisible(false);
}

void WdgSmallPresetHandler::on_rightScrollBtn_pressed()
{
    // Deciding how far beyond the right margin to put the point (10 pixels) was an arbitrary decision
    QPoint beyondRightMargin(10 + antiOOPHack->viewport()->width(), 0);
    antiOOPHack->scrollTo(antiOOPHack->indexAt(beyondRightMargin), QAbstractItemView::EnsureVisible);
    
    deletePresetBtn->setVisible(false);
}

void WdgSmallPresetHandler::on_deletePresetBtn_pressed()
{
    KoResourceItemChooser* veryAntiOOPHack = smallPresetChooser->findChild<KoResourceItemChooser*>();
    veryAntiOOPHack->slotButtonClicked(KoResourceItemChooser::Button_Remove);
}

#include "kis_small_preset_handler.moc"