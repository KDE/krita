/*
 *  SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_preset_selector_strip.h"

#include <KisResourceItemListView.h>
#include <KisResourceItemChooser.h>
#include <brushengine/kis_paintop_registry.h>
#include "kis_config.h"
#include <kis_icon.h>

#include <QAbstractScrollArea>
#include <QMouseEvent>

KisPresetSelectorStrip::KisPresetSelectorStrip(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
    smallPresetChooser->showButtons(false); //loading and saving buttons. don't need these with the brush editor
    smallPresetChooser->setViewMode((KisPresetChooser::ViewMode)KisConfig(true).presetChooserViewMode());
    smallPresetChooser->showTaggingBar(true);
    m_resourceItemView = smallPresetChooser->itemChooser()->itemView();

    /* This is an heuristic to fill smallPresetChooser with only the presets
     * for the paintop that comes selected by default: Pixel Brush. */
    const QString PIXEL_BRUSH_ID = "paintbrush";
    m_currentPaintopID = PIXEL_BRUSH_ID;


    // hide the left and right arrows that are used by the "strip" view by default
    rightScrollBtn->hide();
    leftScrollBtn->hide();

}

KisPresetSelectorStrip::~KisPresetSelectorStrip()
{
}

void KisPresetSelectorStrip::setPresetFilter(const QString& paintOpId)
{
    smallPresetChooser->setPresetFilter(paintOpId);
    if (m_currentPaintopID != paintOpId) {
        m_resourceItemView->scrollTo(m_resourceItemView->model()->index(0, 0));
        m_currentPaintopID = paintOpId;
    }
}


void KisPresetSelectorStrip::on_leftScrollBtn_pressed()
{
    // Deciding how far beyond the left margin (10 pixels) was an arbitrary decision
    QPoint beyondLeftMargin(-10, 0);
    m_resourceItemView->scrollTo(m_resourceItemView->indexAt(beyondLeftMargin), QAbstractItemView::EnsureVisible);
}

void KisPresetSelectorStrip::on_rightScrollBtn_pressed()
{
    // Deciding how far beyond the right margin to put the point (10 pixels) was an arbitrary decision
    QPoint beyondRightMargin(10 + m_resourceItemView->viewport()->width(), 0);
    m_resourceItemView->scrollTo(m_resourceItemView->indexAt(beyondRightMargin), QAbstractItemView::EnsureVisible);
}


void KisPresetSelectorStrip::slotThumbnailMode()
{
    smallPresetChooser->setViewMode(KisPresetChooser::THUMBNAIL); // set to details view by default to see names
    m_resourceItemView = smallPresetChooser->itemChooser()->itemView();
}

void KisPresetSelectorStrip::slotDetailMode()
{
    smallPresetChooser->setViewMode(KisPresetChooser::DETAIL); // set to details view by default to see names
    m_resourceItemView = smallPresetChooser->itemChooser()->itemView();
}


int KisPresetSelectorStrip::iconSize()
{
    return smallPresetChooser->iconSize();
}


void KisPresetSelectorStrip::slotSetIconSize(int size)
{
    smallPresetChooser->setIconSize(size);
}

void KisPresetSelectorStrip::slotSaveIconSize() {
    smallPresetChooser->saveIconSize();
}


