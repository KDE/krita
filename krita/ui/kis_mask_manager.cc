/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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
#include "kis_mask_manager.h"

#include <klocale.h>
#include <kstandardaction.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

#include <KoID.h>

#include "kis_dlg_transformation_effect.h"
#include <kis_undo_adapter.h>
#include <kis_paint_layer.h>
#include "kis_doc2.h"
#include "kis_view2.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_transformation_mask.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"
#include "kis_mask.h"
#include "kis_effect_mask.h"
#include "kis_dlg_adjustment_layer.h"
#include "kis_mask_widgets.h"
#include "kis_selection.h"
#include "kis_pixel_selection.h"

KisMaskManager::KisMaskManager( KisView2 * view)
    : m_view( view )
    , m_activeMask( 0 )
    , m_createTransparencyMask( 0 )
    , m_createFilterMask( 0 )
    , m_createTransformationMask( 0 )
    , m_maskToSelection( 0 )
    , m_maskToLayer( 0 )
    , m_duplicateMask( 0 )
    , m_showMask( 0 )
    , m_enableMask( 0 )
    , m_removeMask( 0 )
    , m_raiseMask( 0 )
    , m_lowerMask( 0 )
    , m_maskToTop( 0 )
    , m_maskToBottom( 0 )
    , m_mirrorMaskX( 0 )
    , m_mirrorMaskY( 0 )
    , m_maskProperties( 0 )
{
}

void KisMaskManager::setup( KActionCollection * actionCollection )
{
    // XXX: Set shortcuts, tooltips, what's this text and statustips!

    m_createTransparencyMask  = new KAction(i18n("Transparency Mask"), this);
    actionCollection->addAction("create_transparency_mask", m_createTransparencyMask );
    connect(m_createTransparencyMask, SIGNAL(triggered()), this, SLOT(createTransparencyMask()));

    m_createFilterMask  = new KAction(i18n("Filter Mask..."), this);
    actionCollection->addAction("create_filter_mask", m_createFilterMask );
    connect(m_createFilterMask, SIGNAL(triggered()), this, SLOT(createFilterMask()));

    m_createTransformationMask  = new KAction(i18n("Transformation Mask..."), this);
    actionCollection->addAction("create_transformation_mask", m_createTransformationMask );
    connect(m_createTransformationMask, SIGNAL(triggered()), this, SLOT(createTransformationMask()));

    m_maskToSelection  = new KAction(i18n("Mask To Selection"), this);
    actionCollection->addAction("create_selection_from_mask", m_maskToSelection );
    connect(m_maskToSelection, SIGNAL(triggered()), this, SLOT(maskToSelection()));

    m_maskToLayer = new KAction( i18n( "Create Layer from Mask..." ), this );
    actionCollection->addAction( "create_layer_from_mask", m_maskToLayer );
    connect( m_maskToLayer, SIGNAL( triggered() ), this, SLOT( maskToLayer() ) );

    m_duplicateMask = new KAction( i18n( "Duplicate Mask" ), this );
    actionCollection->addAction( "duplicate_mask", m_duplicateMask );
    connect( m_duplicateMask, SIGNAL( triggered() ), this, SLOT( duplicateMask() ) );

    m_enableMask = new KToggleAction( i18n( "Enable Mask" ), this );
    actionCollection->addAction( "enable_mask", m_enableMask );
    connect( m_enableMask, SIGNAL( triggered() ), this, SLOT( enableMask() ) );

    m_removeMask  = new KAction(i18n("Remove Mask"), this);
    actionCollection->addAction("remove_mask", m_removeMask );
    connect(m_removeMask, SIGNAL(triggered()), this, SLOT(removeMask()));

    m_showMask  = new KToggleAction(i18n("Show Mask"), this);
    actionCollection->addAction("show_mask", m_showMask );
    connect(m_showMask, SIGNAL(triggered()), this, SLOT(showMask()));

    m_raiseMask = new KAction(i18n("Raise Mask"), this);
    actionCollection->addAction("raise_mask", m_raiseMask );
    connect(m_raiseMask, SIGNAL(triggered()), this, SLOT(raiseMask()));

    m_lowerMask  = new KAction(i18n("Lower Mask"), this);
    actionCollection->addAction("lower_mask", m_lowerMask );
    connect(m_lowerMask, SIGNAL(triggered()), this, SLOT(lowerMask()));

    m_maskToTop  = new KAction(i18n("Move Mask to Top"), this);
    actionCollection->addAction("mask_to_top", m_maskToTop );
    connect(m_maskToTop, SIGNAL(triggered()), this, SLOT(maskToTop()));

    m_maskToBottom = new KAction(i18n("Move Mask to Bottom"), this);
    actionCollection->addAction("mask_to_bottom", m_maskToBottom );
    connect(m_maskToBottom, SIGNAL(triggered()), this, SLOT(maskToBottom()));

    m_mirrorMaskX = new KAction(i18n("Mirror Mask Horizontally"), this);
    actionCollection->addAction("mirror_mask_x", m_mirrorMaskX );
    connect(m_mirrorMaskX, SIGNAL(triggered()), this, SLOT(mirrorMaskX()));

    m_mirrorMaskY  = new KAction(i18n("Mirror Mask Vertically"), this);
    actionCollection->addAction("mirror_mask_y", m_mirrorMaskY );
    connect(m_mirrorMaskY, SIGNAL(triggered()), this, SLOT(mirrorMaskY()));

    m_maskProperties  = new KAction(i18n("Mask Properties"), this);
    actionCollection->addAction("mask_properties", m_maskProperties );
    connect(m_maskProperties, SIGNAL(triggered()), this, SLOT(showMaskProperties()));


}

void KisMaskManager::updateGUI()
{

}

KisMaskSP KisMaskManager::activeMask()
{
    return m_activeMask;
}

KisPaintDeviceSP KisMaskManager::activeDevice()
{
    if ( m_activeMask ) {
        KisSelectionSP selection = m_activeMask->selection();
        if (selection)
            return m_activeMask->selection()->getOrCreatePixelSelection();
    }
    return 0;
}

void KisMaskManager::activateMask( KisMaskSP mask )
{
    m_activeMask = mask;
    emit sigMaskActivated( mask );
}

void KisMaskManager::createTransparencyMask()
{

    KisLayerSP parent = m_view->activeLayer();
    if ( parent ) {
        if ( parent == 0 )
            createTransparencyMask( parent.data(), parent->firstChild() );
        else
            createTransparencyMask( parent.data(), m_activeMask.data() );
    }
}

void KisMaskManager::createTransparencyMask( KisNodeSP parent, KisNodeSP above )
{
    // XXX: if there's a selection, set the selection on the mask
    //      if there's no selection, select everything
    KisMaskSP mask = new KisTransparencyMask();
    mask->setName( i18n( "Transparency Mask" ) ); // XXX:Auto-increment a number here, like with layers
    mask->setActive( true );
    m_view->image()->addNode( mask, parent, above );
    activateMask( mask );
}


void KisMaskManager::createFilterMask()
{
    KisLayerSP parent = m_view->activeLayer();

    if ( parent ) {
        if ( m_activeMask == 0 )
            createFilterMask( parent.data(), parent->firstChild() );
        else
            createFilterMask( parent.data(), m_activeMask.data() );

    }
}

void KisMaskManager::createFilterMask( KisNodeSP parent, KisNodeSP above )
{
    KisLayerSP layer = dynamic_cast<KisLayer*>(parent.data());
    if (! layer )
        return;

    KisPaintDeviceSP dev = layer->projection();

    KisDlgAdjustmentLayer dlg(dev, m_view->image()->nextLayerName(), i18n("New Filter Mask"), m_view, "dlgfiltermask");

    if (dlg.exec() == QDialog::Accepted) {
        KisSelectionSP selection = KisSelectionSP(0);
        if (dev->hasSelection()) {
            selection = dev->selection();
        }
        KisFilterConfiguration * filter = dlg.filterConfiguration();
        QString name = dlg.layerName();

        KisFilterMask * mask = new KisFilterMask();

        mask->setActive( true );
        mask->setFilter( filter );
        mask->setName( name );
        m_view->image()->addNode( mask, parent, above );
        activateMask( mask );
    }
}

void KisMaskManager::createTransformationMask()
{
    KisLayerSP activeLayer = m_view->activeLayer();

    if ( activeLayer ) {
        if ( m_activeMask == 0 )
            createTransformationMask( activeLayer.data(), activeLayer->firstChild() );
        else
            createTransformationMask( activeLayer.data(), m_activeMask.data() );
    }
}

void KisMaskManager::createTransformationMask( KisNodeSP parent, KisNodeSP above )
{
    KisDlgTransformationEffect dlg(QString(), 1.0, 1.0, 0.0, 0.0, 0.0, 0, 0, KoID( "Mitchell" ), m_view);
    if ( dlg.exec() == QDialog::Accepted ) {
        KisTransformationMask * mask = new KisTransformationMask();

        mask->setName( dlg.transformationEffect()->maskName() );
        mask->setXScale( dlg.transformationEffect()->xScale() );
        mask->setYScale( dlg.transformationEffect()->yScale() );
        mask->setXShear( dlg.transformationEffect()->xShear() );
        mask->setYShear( dlg.transformationEffect()->yShear() );
        mask->setRotation( dlg.transformationEffect()->rotation() );
        mask->setXTranslation( dlg.transformationEffect()->moveX() );
        mask->setYTranslation( dlg.transformationEffect()->moveY() );
        mask->setFilterStrategy( dlg.transformationEffect()->filterStrategy() );
        mask->setActive( true );
        m_view->image()->addNode( mask, parent, above );
        activateMask( mask );
    }
}

void KisMaskManager::maskToSelection() {}

void KisMaskManager::maskToLayer() {}

void KisMaskManager::duplicateMask() {}

void KisMaskManager::showMask() {}

void KisMaskManager::enableMask() {}

void KisMaskManager::removeMask() {}

void KisMaskManager::raiseMask() {}

void KisMaskManager::lowerMask() {}

void KisMaskManager::maskToTop() {}

void KisMaskManager::maskToBottom() {}

void KisMaskManager::mirrorMaskX() {}

void KisMaskManager::mirrorMaskY() {}

void KisMaskManager::showMaskProperties() {}

void KisMaskManager::masksUpdated() {}

#include "kis_mask_manager.moc"
