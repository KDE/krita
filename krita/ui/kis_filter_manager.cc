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
#include <q3frame.h>
#include <qcursor.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <kmessagebox.h>
#include <kguiitem.h>

#include "kaction.h"

#include "kis_part_layer.h"
#include "kis_id.h"
#include "kis_view.h"
#include "kis_doc.h"
#include "kis_filter.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_filter_manager.h"
#include "kis_filter_config_widget.h"
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
#include "kis_transaction.h"
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
    //delete m_reapplyAction;
    //delete m_lastFilterConfig;
    //delete m_filterMapper;
}

void KisFilterManager::setup(KActionCollection * ac)
{
    KisFilterSP f;
    int i = 0;

    // Only create the submenu's we've actually got filters for.
    // XXX: Make this list extensible after 1.5

    KActionMenu * other = 0;
    KActionMenu * am = 0;
    
    m_filterList = KisFilterRegistry::instance()->listKeys();
    
    for ( KisIDList::Iterator it = m_filterList.begin(); it != m_filterList.end(); ++it ) {
        f = KisFilterRegistry::instance()->get(*it);
        if (!f) break;
        
        QString s = f->menuCategory();
        if (s == "adjust" && !m_filterActionMenus.find("adjust")) {
            am = new KActionMenu(i18n("Adjust"), ac, "adjust_filters");
            m_filterActionMenus.insert("adjust", am);
        }

        else if (s == "artistic" && !m_filterActionMenus.find("artistic")) {
            am = new KActionMenu(i18n("Artistic"), ac, "artistic_filters");
            m_filterActionMenus.insert("artistic", am);
        }

        else if (s == "blur" && !m_filterActionMenus.find("blur")) {
            am = new KActionMenu(i18n("Blur"), ac, "blur_filters");
            m_filterActionMenus.insert("blur", am);
        }

        else if (s == "colors" && !m_filterActionMenus.find("colors")) {
            am = new KActionMenu(i18n("Colors"), ac, "color_filters");
            m_filterActionMenus.insert("colors", am);
        }

        else if (s == "decor" && !m_filterActionMenus.find("decor")) {
            am = new KActionMenu(i18n("Decor"), ac, "decor_filters");
            m_filterActionMenus.insert("decor", am);
        }

        else if (s == "edge" && !m_filterActionMenus.find("edge")) {
            am = new KActionMenu(i18n("Edge Detection"), ac, "edge_filters");
            m_filterActionMenus.insert("edge", am);
        }

        else if (s == "emboss" && !m_filterActionMenus.find("emboss")) {
            am = new KActionMenu(i18n("Emboss"), ac, "emboss_filters");
            m_filterActionMenus.insert("emboss", am);
        }

        else if (s == "enhance" && !m_filterActionMenus.find("enhance")) {
            am = new KActionMenu(i18n("Enhance"), ac, "enhance_filters");
            m_filterActionMenus.insert("enhance", am);
        }

        else if (s == "map" && !m_filterActionMenus.find("map")) {
            am = new KActionMenu(i18n("Map"), ac, "map_filters");
            m_filterActionMenus.insert("map", am);
        }

        else if (s == "nonphotorealistic" && !m_filterActionMenus.find("nonphotorealistic")) {
            am = new KActionMenu(i18n("Non-photorealistic"), ac, "nonphotorealistic_filters");
            m_filterActionMenus.insert("nonphotorealistic", am);
        }

        else if (s == "other" && !m_filterActionMenus.find("other")) {
            other = new KActionMenu(i18n("Other"), ac, "misc_filters");
            m_filterActionMenus.insert("other", am);
        }
        
    }

    m_reapplyAction = new KAction(i18n("Apply Filter Again"),
                "Ctrl+Shift+F",
                this, SLOT(slotApply()),
                ac, "filter_apply_again");
    
    m_reapplyAction->setEnabled(false);

    f = 0;
    i = 0;
    for ( KisIDList::Iterator it = m_filterList.begin(); it != m_filterList.end(); ++it ) {
        f = KisFilterRegistry::instance()->get(*it);

        if (!f) break;

        // Create action
        KAction * a = new KAction(f->menuEntry(), 0, m_filterMapper, SLOT(map()), ac,
                                  QString("krita_filter_%1").arg((*it) . id()).ascii());

        // Add action to the right submenu
        KActionMenu * m = m_filterActionMenus.find( f->menuCategory() );
        if (m) {
            m->insert(a);
        }
        else {
            if (!other) {
                other = new KActionMenu(i18n("Other"), ac, "misc_filters");
                m_filterActionMenus.insert("other", am);
            }
            other->insert(a);
        }

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

    KisPartLayer * partLayer = dynamic_cast<KisPartLayer*>(layer.data());
    
    bool enable =  !(layer->locked() || !layer->visible() || partLayer);
    KisPaintLayerSP player = KisPaintLayerSP(dynamic_cast<KisPaintLayer*>( layer.data()));
    if(!player)
    {
        enable = false;
    }
    m_reapplyAction->setEnabled(m_lastFilterConfig);
    if (m_lastFilterConfig)
        m_reapplyAction->setText(i18n("Apply Filter Again") + ": " 
            + KisFilterRegistry::instance()->get(m_lastFilterConfig->name())->id().name());
    else
        m_reapplyAction->setText(i18n("Apply Filter Again"));
    
    KAction * a;
    int i = 0;
    for (a = m_filterActions.first(); a; a = m_filterActions.next() , i++) {
        KisFilterSP filter = KisFilterRegistry::instance()->get(m_filterList[i]);
        if(player && filter->workWith( player->paintDevice()->colorSpace()))
        {
            a->setEnabled(enable);
        } else {
            a->setEnabled(false);
        }
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

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return false;

    QApplication::setOverrideCursor( Qt::WaitCursor );

    //Apply the filter
    m_lastFilterConfig = m_lastFilter->configuration(m_lastWidget);

    QRect r1 = dev->extent();
    QRect r2 = img->bounds();

    // Filters should work only on the visible part of an image.
    QRect rect = r1.intersect(r2);

    if (dev->hasSelection()) {
        QRect r3 = dev->selection()->selectedExactRect();
        rect = rect.intersect(r3);
    }

    m_lastFilter->enableProgress();

    m_view->progressDisplay()->setSubject(m_lastFilter, true, true);
    m_lastFilter->setProgressDisplay( m_view->progressDisplay());

    KisTransaction * cmd = 0;
    if (img->undo()) cmd = new KisTransaction(m_lastFilter->id().name(), dev);
    
    m_lastFilter->process(dev, dev, m_lastFilterConfig, rect);
    m_reapplyAction->setEnabled(m_lastFilterConfig);
    if (m_lastFilterConfig)
        m_reapplyAction->setText(i18n("Apply Filter Again") + ": "
            + KisFilterRegistry::instance()->get(m_lastFilterConfig->name())->id().name());

    else
        m_reapplyAction->setText(i18n("Apply Filter Again"));
    
    if (m_lastFilter->cancelRequested()) {
        delete m_lastFilterConfig;
        if (cmd) {
            cmd->unexecute();
            delete cmd;
        }
        m_lastFilter->disableProgress();
        QApplication::restoreOverrideCursor();
        return false;

    } else {
        if (dev->parentLayer()) dev->parentLayer()->setDirty(rect);
        m_doc->setModified(true);
        if (img->undo() && cmd) img->undoAdapter()->addCommand(cmd);
        m_lastFilter->disableProgress();
        QApplication::restoreOverrideCursor();
        return true;
    }
}

void KisFilterManager::slotApplyFilter(int i)
{
    KisPreviewDialog * oldDialog = m_lastDialog;
    KisFilterConfiguration * oldConfig = m_lastFilterConfig;
    KisFilter * oldFilter = m_lastFilter;

    m_lastFilter = KisFilterRegistry::instance()->get(m_filterList[i]).data();

    if (!m_lastFilter) {
        m_lastFilter = oldFilter;
        return;
    }

    KisImageSP img = m_view->currentImg();
    if (!img) return;

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev) return;

    if (dev->colorSpace()->willDegrade(m_lastFilter->colorSpaceIndependence())) {
        // Warning bells!
        if (m_lastFilter->colorSpaceIndependence() == TO_LAB16) {
            if (KMessageBox::warningContinueCancel(m_view,
                                               i18n("The %1 filter will convert your %2 data to 16-bit L*a*b* and vice versa. ")
                                                       .arg(m_lastFilter->id().name())
                                                       .arg(dev->colorSpace()->id().name()),
                                               i18n("Filter Will Convert Your Layer Data"),
                                               KGuiItem(i18n("Continue")),
                                               "lab16degradation") != KMessageBox::Continue) return;

        }
        else if (m_lastFilter->colorSpaceIndependence() == TO_RGBA8) {
            if (KMessageBox::warningContinueCancel(m_view,
                                               i18n("The %1 filter will convert your %2 data to 8-bit RGBA and vice versa. ")
                                                       .arg(m_lastFilter->id().name())
                                                       .arg(dev->colorSpace()->id().name()),
                                               i18n("Filter Will Convert Your Layer Data"),
                                               KGuiItem(i18n("Continue")),
                                               "rgba8degradation") != KMessageBox::Continue) return;
        }
    }

    m_lastFilter->disableProgress();

    // Create the config dialog
    m_lastDialog = new KisPreviewDialog(m_view, m_lastFilter->id().name().ascii(), true, m_lastFilter->id().name());
    Q_CHECK_PTR(m_lastDialog);
    m_lastWidget = m_lastFilter->createConfigurationWidget( (QWidget*)m_lastDialog->container(), dev );


    if( m_lastWidget != 0)
    {
        connect(m_lastWidget, SIGNAL(sigPleaseUpdatePreview()), this, SLOT(slotConfigChanged()));

        m_lastDialog->previewWidget()->slotSetDevice( dev );

        connect(m_lastDialog->previewWidget(), SIGNAL(updated()), this, SLOT(refreshPreview()));

        Q3GridLayout *widgetLayout = new Q3GridLayout((QWidget *)m_lastDialog->container(), 1, 1);

        widgetLayout->addWidget(m_lastWidget, 0 , 0);

        m_lastDialog->container()->setMinimumSize(m_lastWidget->minimumSize());

        refreshPreview();

        if(m_lastDialog->exec() == QDialog::Rejected )
        {
            delete m_lastDialog;
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
    {
        refreshPreview();
    } else {
        m_lastDialog->previewWidget()->needUpdate();
    }
}


void KisFilterManager::refreshPreview( )
{
    if( m_lastDialog == 0 )
        return;

    KisPaintDeviceSP dev = m_lastDialog->previewWidget()->getDevice();
    if (!dev) return;

    KisFilterConfiguration* config = m_lastFilter->configuration(m_lastWidget);

    QRect rect = dev->extent();
    KisTransaction cmd("Temporary transaction", dev);
    m_lastFilter->process(dev, dev, config, rect);
    m_lastDialog->previewWidget()->slotUpdate();
    cmd.unexecute();
}


#include "kis_filter_manager.moc"
