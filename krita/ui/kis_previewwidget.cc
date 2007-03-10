/*
 *  kis_previewwidget.cc - part of Krita
 *
 *  Copyright (c) 2001 John Califf  <jwcaliff@compuzone.net>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Ben Schleimer <bensch128@yahoo.com>
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

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qcolor.h>
#include <qgroupbox.h>
#include <qcursor.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <kpushbutton.h>

#include <kis_cursor.h>
#include <kis_colorspace.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_config.h>
#include <kis_filter_strategy.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_meta_registry.h>
#include <kis_painter.h>
#include <kis_profile.h>
#include <kis_types.h>
#include <kis_undo_adapter.h>
#include <kis_label_progress.h>

#include "kis_previewwidgetbase.h"
#include "kis_previewwidget.h"
#include "imageviewer.h"


KisPreviewWidget::KisPreviewWidget( QWidget* parent, const char* name )
    : PreviewWidgetBase( parent, name )
    , m_autoupdate(true)
    , m_previewIsDisplayed(true)
    , m_scaledOriginal()
    , m_dirtyOriginal(true)
    , m_origDevice(NULL)
    , m_scaledPreview()
    , m_dirtyPreview(true)
    , m_previewDevice(NULL)
    , m_zoom(-1.0)
    , m_profile(NULL)
    , m_filterTimer(new QTimer(this, "filterTimer"))
    , m_filterX(0)
    , m_filterY(0)
    , m_runFilter(false)
    , m_filter(NULL)
    , m_filterConfig(NULL)
    , m_progress( 0 )
{
    btnZoomIn->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "viewmag+", KIcon::MainToolbar, 16 ));
    connect(btnZoomIn, SIGNAL(clicked()), this, SLOT(zoomIn()));
    btnZoomOut->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "viewmag-", KIcon::MainToolbar, 16 ));
    connect(btnZoomOut, SIGNAL(clicked()), this, SLOT(zoomOut()));
    btnUpdate->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "reload", KIcon::MainToolbar, 16 ));
    connect(btnUpdate, SIGNAL(clicked()), this, SLOT(forceUpdate()));

    connect(radioBtnPreview, SIGNAL(toggled(bool)), this, SLOT(setPreviewDisplayed(bool)));

    connect(checkBoxAutoUpdate, SIGNAL(toggled(bool)), this, SLOT(slotSetAutoUpdate(bool)));
    btnZoomOneToOne->setIconSet(KGlobal::instance()->iconLoader()->loadIconSet( "viewmag1", KIcon::MainToolbar, 16 ));
    connect(btnZoomOneToOne, SIGNAL(clicked()), this, SLOT(zoomOneToOne()));

    connect(m_filterTimer, SIGNAL(timeout()), this, SLOT(filterNextBlock()));

    m_progress = new KisLabelProgress(frmProgress);
    m_progress->setMaximumWidth(225);
    m_progress->setMinimumWidth(225);
    m_progress->setMaximumHeight(fontMetrics().height() );
    QVBoxLayout *vbox = new QVBoxLayout( frmProgress );
    vbox->addWidget(m_progress);
    m_progress->hide();

/*    kToolBar1->insertLineSeparator();
    kToolBar1->insertButton("reload",2, true, i18n("Update"));
    connect(kToolBar1->getButton(2),SIGNAL(clicked()),this,SLOT(forceUpdate()));

    kToolBar1->insertButton("",3, true, i18n("Auto Update"));
    connect(kToolBar1->getButton(3),SIGNAL(clicked()),this,SLOT(toggleAutoUpdate()));

    kToolBar1->insertButton("",4, true, i18n("Switch"));
    connect(kToolBar1->getButton(4),SIGNAL(clicked()),this,SLOT(toggleImageDisplayed()));*/
// these currently don't yet work, reenable when they do work :)  (TZ-12-2005)
// TODO reenable these
//   kToolBar1->insertButton("",5, true, i18n("Popup Original and Preview"));
}

KisPreviewWidget::~KisPreviewWidget()
{
    if(m_runFilter)
    {
        // Make sure we clean up properly
        QApplication::restoreOverrideCursor();
        m_filterTimer->stop();
    }
}

void KisPreviewWidget::forceUpdate()
{
    if(m_previewIsDisplayed)
    {
        m_groupBox->setTitle(m_origDevice->name());
        emit updated();
    }
}

void KisPreviewWidget::slotSetDevice(KisPaintDeviceSP dev)
{
    Q_ASSERT( dev );

    if (!dev) return;

    m_origDevice = dev;
    m_previewDevice = dev;

    KisConfig cfg;
    QString monitorProfileName = cfg.monitorProfile();
    m_profile = KisMetaRegistry::instance()->csRegistry()->getProfileByName(monitorProfileName);

    QRect r = dev->exactBounds();

    m_groupBox->setTitle(i18n("Preview: ") + dev->name());
    m_previewIsDisplayed = true;

    m_zoom = -1.0;
    zoomChanged(double(m_preview->width()) / double(r.width()) );
}

void KisPreviewWidget::slotUpdate()
{
    // Always assume that the previewDevice was modified
    m_dirtyPreview = true;

    updateInternal();
}

void KisPreviewWidget::updateInternal()
{
    QSize r = m_origDevice->extent().size();
    int w = r.width(), h = r.height();
    int sw = int(ceil(w * m_zoom)), sh = int(ceil(h * m_zoom));

    QApplication::setOverrideCursor(KisCursor::waitCursor());    

    if(m_previewIsDisplayed)
    {
        // Scale the preview
        if(m_dirtyPreview)
        {
            m_dirtyPreview = false;
            m_scaledPreview = m_previewDevice->convertToQImage(m_profile, 0, 0, w, h);
            m_scaledPreview = m_scaledPreview.scale(sw,sh, QImage::ScaleMax); // Use scale instead of smoothScale for speed up
        }
        m_preview->setImage(m_scaledPreview);
    } else
    {
        // Scale the preview
        if(m_dirtyOriginal)
        {
            m_dirtyOriginal = false;
            m_scaledOriginal = m_origDevice->convertToQImage(m_profile, 0, 0, w, h);
            m_scaledOriginal = m_scaledOriginal.scale(sw,sh, QImage::ScaleMax); // Use scale instead of smoothScale for speed up
        }
        m_preview->setImage(m_scaledOriginal);
    }

    QApplication::restoreOverrideCursor();
}

void KisPreviewWidget::slotSetAutoUpdate(bool set) {
    m_autoupdate = set;
}

void KisPreviewWidget::wheelEvent(QWheelEvent * e)
{
    if (e->delta() > 0) {
        zoomIn();
    } else {
        zoomOut();
    }
    e->accept();
}

void KisPreviewWidget::setPreviewDisplayed(bool v)
{
    if(v != m_previewIsDisplayed)
    {
        m_previewIsDisplayed = v;
        if(m_previewIsDisplayed) {
            m_groupBox->setTitle(i18n("Preview: ") + m_origDevice->name());
        } else {
            m_groupBox->setTitle(i18n("Original: ") + m_origDevice->name());
        }
        updateInternal();
    }
}

void KisPreviewWidget::needUpdate()
{
    if(m_previewIsDisplayed)
        m_groupBox->setTitle(i18n("Preview (needs update)"));
}

bool KisPreviewWidget::getAutoUpdate()  const {
    return m_autoupdate;
}

void KisPreviewWidget::zoomChanged(const double zoom)
{
    // constrain the zoom
    double tZoom = zoom;
    if(zoom <= 1./8.) { tZoom = 1./8.; }
    if(zoom > 8.) { tZoom = 8.; }

    if(tZoom != m_zoom) 
    {
        m_zoom = tZoom;
        m_dirtyOriginal = true;
        m_dirtyPreview = true;
//         if(m_previewIsDisplayed)
//         {
//             updated();
//         } else
        {
            updateInternal();
        }
    }
}

void KisPreviewWidget::zoomIn() {
    zoomChanged(m_zoom * 1.1);
}

void KisPreviewWidget::zoomOut() {
    zoomChanged(m_zoom / 1.1);
}

void KisPreviewWidget::zoomOneToOne() {
    zoomChanged(1.0);
}

void KisPreviewWidget::runFilter(KisFilter * filter, KisFilterConfiguration * config)
{
    if(!filter) return;
    if(!config) return;

    // Rebuild the preview device for the filtering
    m_previewDevice = new KisPaintDevice(*m_origDevice);
    if(!m_previewDevice) return;

    m_filter = filter;
    m_filterConfig = config;

    // Setup a timer to filter only one chunk at a time.
    // Alternatively, setup threads here to take advantage of multiple processors.
    
    // Setup the progress display
    m_filter->enableProgress();
    m_progress->setSubject(m_filter, true, true);
    m_filter->setProgressDisplay(m_progress);
    // QApplication::setOverrideCursor(KisCursor::waitCursor(), true);
    if(filter->supportsThreading())
    {
        m_runFilter = true;
        m_filterX = m_filterY = 0;
        // NOTE: do a single shot to avoid recursion problems with the event queue 
        m_filterTimer->start(0, true);
    } else
    {
        QRect rect = m_origDevice->extent();
        m_filter->process(m_origDevice, m_previewDevice, config, rect);
        m_filter->disableProgress();
        QApplication::restoreOverrideCursor();
        slotUpdate();
    }
}

#define FILTER_BLOCK_SIZE 128

void KisPreviewWidget::filterNextBlock()
{
    // kdDebug(12345) << "KisPreviewWidget::filterNextBlock: called\n";
    if(m_runFilter == false || m_filter->cancelRequested())
    {
        m_filterTimer->stop();
        m_filter->disableProgress();
        QApplication::restoreOverrideCursor();
        // only update once all of the chunks are done.
        // slotUpdate();
        return;
    }
    m_filter->enableProgress();
    m_progress->setSubject(m_filter, true, true);
    m_filter->setProgressDisplay(m_progress);
    QRect rect(m_filterX, m_filterY, FILTER_BLOCK_SIZE, FILTER_BLOCK_SIZE);
    // clip the rect to render
    rect = rect.intersect(m_origDevice->extent());

    m_filter->process(m_origDevice, m_previewDevice, m_filterConfig, rect);
    // Update after each chunk is done
    slotUpdate();
    m_filterX += FILTER_BLOCK_SIZE;
    if(m_filterX >= m_origDevice->extent().width())
    {
        m_filterX = 0;
        m_filterY += FILTER_BLOCK_SIZE;
        if(m_filterY >= m_origDevice->extent().height())
        {
            // Stop once we are done
            m_runFilter = false; 
        }
    }
    // NOTE: do a single shot to avoid recursion problems with the event queue 
    m_filterTimer->start(0, true);
}


#include "kis_previewwidget.moc"
