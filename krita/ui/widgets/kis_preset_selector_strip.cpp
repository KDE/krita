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

#include "kis_preset_selector_strip.h"

#include "KoResourceModel.h"
#include "KoResourceItemView.h"
#include "KoResourceItemChooser.h"
#include "kis_paintop_registry.h"

#include <KoIcon.h>

#include <QAbstractScrollArea>
#include <QMouseEvent>
#include <QTimer>

KisPresetSelectorStrip::KisPresetSelectorStrip(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
    smallPresetChooser->showButtons(false);
    smallPresetChooser->setViewMode(KisPresetChooser::STRIP);
    m_antiOOPHack = smallPresetChooser->findChild<KoResourceItemView*>();
    m_antiOOPHack->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_antiOOPHack->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_refresher = new QTimer(this);
    m_refresher->setSingleShot(true);
    
    /* This is an heuristic to fill smallPresetChooser with only the presets
     * for the paintop that comes selected by default: Pixel Brush. */
    const QString PIXEL_BRUSH_ID = "paintbrush";
    smallPresetChooser->setPresetFilter(KoID(PIXEL_BRUSH_ID));

    deletePresetBtn->setIcon(koIcon("trash-empty"));
    deletePresetBtn->setVisible(true);

    connect(smallPresetChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(prepareDeleteButton()));
    connect(smallPresetChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(startRefreshingTimer()));
    connect(m_refresher, SIGNAL(timeout()), this, SLOT(repaintDeleteButton()));
}

KisPresetSelectorStrip::~KisPresetSelectorStrip()
{
    delete m_refresher;
}

void KisPresetSelectorStrip::showEvent(QShowEvent* event)
{
    deletePresetBtn->hide();
    QWidget::showEvent(event);
}

void KisPresetSelectorStrip::currentPaintopChanged(QString paintOpID)
{
    foreach (KoID paintOp, KisPaintOpRegistry::instance()->listKeys()) {
        if (paintOp.id() == paintOpID) {
            smallPresetChooser->setPresetFilter(paintOp);
            break;
        }
    }
    deletePresetBtn->hide();
}

void KisPresetSelectorStrip::startRefreshingTimer()
{
    // Estimated time it takes for the ResourceView to scroll when a widget
    // that is only partially visible becomes visible
    m_refresher->start(450);
}

void KisPresetSelectorStrip::repaintDeleteButton()
{
    if (deletePresetBtn->isVisible()) {
        prepareDeleteButton();
    }
}

void KisPresetSelectorStrip::prepareDeleteButton()
{
    const quint8 HEURISTIC_OFFSET = 7;  // This number is just conjured out of the nether to make
                                        // things look good
    quint16 buttonWidth     = deletePresetBtn->width();
    quint16 buttonHeight    = deletePresetBtn->height();
    quint16 columnWidth     = m_antiOOPHack->columnWidth(0);  // All columns assumed equal in width
    quint16 currentColumn   = m_antiOOPHack->currentIndex().column();
    quint16 rowHeight       = m_antiOOPHack->rowHeight(0);    // There is only 1 row in this widget
    quint16 yPos            = rowHeight - deletePresetBtn->height() + HEURISTIC_OFFSET;
    quint16 xPos            = m_antiOOPHack->columnViewportPosition(currentColumn)
                              + columnWidth + HEURISTIC_OFFSET - buttonWidth;
    
    deletePresetBtn->setGeometry(xPos, yPos, buttonWidth, buttonHeight);
    deletePresetBtn->setVisible(true);
}

void KisPresetSelectorStrip::on_leftScrollBtn_pressed()
{
    // Deciding how far beyond the left margin (10 pixels) was an arbitrary decision
    QPoint beyondLeftMargin(-10, 0);
    m_antiOOPHack->scrollTo(m_antiOOPHack->indexAt(beyondLeftMargin), QAbstractItemView::EnsureVisible);
    
    deletePresetBtn->setVisible(false);
}

void KisPresetSelectorStrip::on_rightScrollBtn_pressed()
{
    // Deciding how far beyond the right margin to put the point (10 pixels) was an arbitrary decision
    QPoint beyondRightMargin(10 + m_antiOOPHack->viewport()->width(), 0);
    m_antiOOPHack->scrollTo(m_antiOOPHack->indexAt(beyondRightMargin), QAbstractItemView::EnsureVisible);
    
    deletePresetBtn->setVisible(false);
}

void KisPresetSelectorStrip::on_deletePresetBtn_clicked()
{
    KoResourceItemChooser* veryAntiOOPHack = smallPresetChooser->findChild<KoResourceItemChooser*>();
    veryAntiOOPHack->slotButtonClicked(KoResourceItemChooser::Button_Remove);
    deletePresetBtn->hide();
    smallPresetChooser->updateViewSettings();
}

#include "kis_preset_selector_strip.moc"
