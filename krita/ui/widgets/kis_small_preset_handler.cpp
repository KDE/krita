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
#include "kis_paintop_registry.h"

#include <QAbstractScrollArea>

WdgSmallPresetHandler::WdgSmallPresetHandler(QWidget* parent)
                      : QWidget(parent)
{
    setupUi(this);
    smallPresetChooser->showButtons(false);
    smallPresetChooser->setViewMode(KisPresetChooser::STRIP);
    antiOOPHack = smallPresetChooser->findChild<KoResourceItemView*>();
    antiOOPHack->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    antiOOPHack->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /* This is an heuristic to fill smallPresetChooser with only the presets
     * for the paintop that comes selected by default: Pixel Brush.
     * TODO this must be replaced by a more correct approach.
     */
    const QString PIXEL_BRUSH_ID = "paintbrush";
    smallPresetChooser->setPresetFilter(KoID(PIXEL_BRUSH_ID));
}

void WdgSmallPresetHandler::currentPaintopChanged(QString paintOpID)
{
    foreach(KoID paintOp, KisPaintOpRegistry::instance()->listKeys() ) {
        if (paintOp.id() == paintOpID) {
            smallPresetChooser->setPresetFilter(paintOp);
            break;
        }
    }
}

void WdgSmallPresetHandler::on_leftScrollBtn_pressed()
{
    // Deciding how far beyond the left margin (10 pixels) was an arbitrary decision
    QPoint beyondLeftMargin(-10, 0);
    antiOOPHack->scrollTo(antiOOPHack->indexAt(beyondLeftMargin), QAbstractItemView::EnsureVisible);
}

void WdgSmallPresetHandler::on_rightScrollBtn_pressed()
{
    // Deciding how far beyond the right margin to put the point (3 pixels) was an arbitrary decision
    QPoint beyondRightMargin(3 + antiOOPHack->viewport()->width(), 0);
    antiOOPHack->scrollTo(antiOOPHack->indexAt(beyondRightMargin), QAbstractItemView::EnsureVisible);
}

#include "kis_small_preset_handler.moc"