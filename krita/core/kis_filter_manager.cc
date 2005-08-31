/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#include "qsignalmapper.h"
#include <qlayout.h>
#include <qframe.h>
#include <qcursor.h>
#include <qapplication.h>

#include "kaction.h"

#include "kis_id.h"
#include "kis_view.h"
#include "kis_doc.h"
#include "kis_filter.h"
#include "kis_layer.h"
#include "kis_filter_manager.h"
#include "kis_previewwidget.h"
#include "kis_previewdialog.h"
#include "kis_filter_registry.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_previewdialog.h"
#include "kis_previewwidget.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_id.h"
#include "kis_canvas_subject.h"
#include "kis_doc.h"
#include <kis_progress_display_interface.h>

KisFilterManager::KisFilterManager(KisView * view, KisDoc * doc)
    : m_view(view),
    m_doc(doc)
{
    // XXX: Store & restore last filter & last filter configuration in session settings
    m_reapplyAction = 0;
    m_lastFilterConfig = 0;
    m_lastDialog = 0;
    m_lastFilter = 0;
    m_lastWidget = 0;
    
    m_filterMapper = new QSignalMapper(this);
    
    connect(m_filterMapper, SIGNAL(mapped(int)), this, SLOT(slotApplyFilter(int)));

}

KisFilterManager::~KisFilterManager()
{
    delete m_reapplyAction;
    delete m_lastFilterConfig;
    delete m_filterMapper;
}

void KisFilterManager::setup(KActionCollection * ac)
{

    KActionMenu * am = new KActionMenu(i18n("Adjust"), ac, "adjust_filters");
    m_filterActionMenus.insert("adjust", am);

    am = new KActionMenu(i18n("Artistic"), ac, "artistic_filters");
    m_filterActionMenus.insert("artistic", am);

    am = new KActionMenu(i18n("Blur"), ac, "blur_filters");
    m_filterActionMenus.insert("blur", am);

    am = new KActionMenu(i18n("Colors"), ac, "color_filters");
    m_filterActionMenus.insert("colors", am);

    am = new KActionMenu(i18n("Decor"), ac, "decor_filters");
    m_filterActionMenus.insert("decor", am);

    am = new KActionMenu(i18n("Edge detection"), ac, "edge_filters");
    m_filterActionMenus.insert("edge", am);

    am = new KActionMenu(i18n("Emboss"), ac, "emboss_filters");
    m_filterActionMenus.insert("emboss", am);

    am = new KActionMenu(i18n("Enhance"), ac, "enhance_filters");
    m_filterActionMenus.insert("enhance", am);

    am = new KActionMenu(i18n("Map"), ac, "map_filters");
    m_filterActionMenus.insert("map", am);    

    am = new KActionMenu(i18n("Other"), ac, "misc_filters");
    m_filterActionMenus.insert("", am);
    
    m_reapplyAction = new KAction(i18n("Apply filter again"),
                "Ctrl+Shift+J",
                this, SLOT(slotApply()),
                ac, "filter_apply_again");


    m_filterList = KisFilterRegistry::instance()->listKeys();
    KisFilter * f;
    int i = 0;
    for ( KisIDList::Iterator it = m_filterList.begin(); it != m_filterList.end(); ++it ) {
        f = KisFilterRegistry::instance()->get(*it);

        if (!f) break;

        // Create action
        KAction * a = new KAction(f->menuEntry(), 0, m_filterMapper, SLOT(map()), ac);
        
        // Add action to the right submenu
        m_filterActionMenus.find( f->menuCategory() )->insert(a);
        
        // Add filter to list of filters for mapper
        m_filterMapper->setMapping( a, i );

        m_filterActions.append( a );
        ++i;
    }

}

void KisFilterManager::updateGUI()
{
    KisImageSP img = m_view->currentImg();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    bool enable =  !(layer->locked() || !layer->visible());

    m_reapplyAction->setEnabled(enable);

    KAction * a;
    for (a = m_filterActions.first(); a; a = m_filterActions.next()) {
        a->setEnabled(enable);
    }
    
}

void KisFilterManager::slotApply()
{
    apply();
}

bool KisFilterManager::apply()
{
    if (!m_lastFilter) return false;

    KisImageSP img = m_view->currentImg();
    if (!img) return false;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return false;
    
    QApplication::setOverrideCursor( Qt::waitCursor );

    //Apply the filter
    m_lastFilterConfig = m_lastFilter->configuration(m_lastWidget, layer.data());

    QRect r1 = layer -> extent();
    QRect r2 = img -> bounds();

    // Filters should work only on the visible part of an image.
    QRect rect = r1.intersect(r2);

    if (layer->hasSelection()) {
        QRect r3 = layer->selection()->selectedExactRect();
        rect = rect.intersect(r3);
    }

    m_lastFilter->enableProgress();

    m_view->progressDisplay()->setSubject(m_lastFilter, true, true);
    m_lastFilter->setProgressDisplay( m_view->progressDisplay());
    
    KisTransaction * cmd = new KisTransaction(m_lastFilter->id().name(), layer.data());
    Q_CHECK_PTR(cmd);
    m_lastFilter->process((KisPaintDeviceImplSP)layer, (KisPaintDeviceImplSP)layer, m_lastFilterConfig, rect);

    if (m_lastFilter->cancelRequested()) {
        delete m_lastFilterConfig;
        cmd -> unexecute();
        delete cmd;
        return false;
    } else {
        img->undoAdapter()->addCommand(cmd);
        m_doc->setModified(true);
        img->notify();
    }

    m_lastFilter->disableProgress();
    
    QApplication::restoreOverrideCursor();
    
    return true;
}

void KisFilterManager::slotApplyFilter(int i)
{
    KisPreviewDialog * oldDialog = m_lastDialog;
    KisFilterConfiguration * oldConfig = m_lastFilterConfig;
    KisFilter * oldFilter = m_lastFilter;

    kdDebug() << "With index " << i << " found filter: " << m_filterList[i].name() << "\n";
    m_lastFilter = KisFilterRegistry::instance()->get(m_filterList[i]);

    if (!m_lastFilter) {
        m_lastFilter = oldFilter;
        return;
    }

    KisImageSP img = m_view->currentImg();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    m_lastFilter->disableProgress();

    // Create the config dialog
    m_lastDialog = new KisPreviewDialog(m_view, m_lastFilter->id().name().ascii(), true, m_lastFilter->id().name());
    Q_CHECK_PTR(m_lastDialog);
    m_lastWidget = m_lastFilter->createConfigurationWidget( (QWidget*)m_lastDialog->container(), layer.data() );

    
    if( m_lastWidget != 0)
    {
        connect(m_lastWidget, SIGNAL(sigPleaseUpdatePreview()), this, SLOT(slotConfigChanged()));
    
        m_lastDialog->previewWidget()->slotSetLayer( layer );

        connect(m_lastDialog->previewWidget(), SIGNAL(updated()), this, SLOT(refreshPreview()));
        
        
        QGridLayout *widgetLayout = new QGridLayout((QWidget *)m_lastDialog->container(), 1, 1);
        
        widgetLayout -> addWidget(m_lastWidget, 0 , 0);
        
        m_lastDialog->container()->setMinimumSize(m_lastWidget->minimumSize());
        
        refreshPreview();
        
        if(m_lastDialog->exec() == QDialog::Rejected )
        {
            delete m_lastDialog; // XXX: Can I do this? It's too hot to think.
            m_lastDialog = oldDialog;
            m_lastFilter = oldFilter;
            return;
        }
    }

    if (!apply()) {
        m_lastFilterConfig = oldConfig;
        m_lastDialog = oldDialog;
        m_lastFilter = oldFilter;
    }

}

void KisFilterManager::slotConfigChanged()
{
    if( m_lastDialog == 0 )
        return;
    if(m_lastDialog->previewWidget()->getAutoUpdate())
        refreshPreview();
}


void KisFilterManager::refreshPreview( )
{
    if( m_lastDialog == 0 )
        return;
    
    m_lastDialog -> previewWidget() -> slotRenewLayer();
    
    KisLayerSP layer = m_lastDialog -> previewWidget()->getLayer();

    KisFilterConfiguration* config = m_lastFilter->configuration(m_lastWidget, layer.data());
    
    QRect rect = layer -> extent();
    m_lastFilter->process((KisPaintDeviceImplSP) layer, (KisPaintDeviceImplSP) layer, config, rect);
    m_lastDialog->previewWidget() -> slotUpdate();
}


#include "kis_filter_manager.moc"
