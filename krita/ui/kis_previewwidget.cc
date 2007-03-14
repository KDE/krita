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
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_meta_registry.h>
#include <kis_painter.h>
#include <kis_profile.h>
#include <kis_types.h>
#include <kis_undo_adapter.h>
#include <kis_label_progress.h>
#include <kis_selection.h>
#include <kis_transform_worker.h>

#include "kis_previewwidgetbase.h"
#include "kis_previewwidget.h"
#include "imageviewer.h"

static const int ZOOM_PAUSE = 100;
static const int FILTER_PAUSE = 500;
static const double ZOOM_FACTOR = 1.1;

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
    , m_scaledImage(NULL)
    , m_filterZoom(1.0)
    , m_zoom(-1.0)
    , m_profile(NULL)
    , m_progress( 0 )
    , m_zoomTimer(new QTimer(this))
    , m_filterTimer(new QTimer(this))
    , m_firstFilter(true)
    , m_firstZoom(true)
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

    m_progress = new KisLabelProgress(frmProgress);
    m_progress->setMaximumWidth(225);
    m_progress->setMinimumWidth(225);
    m_progress->setMaximumHeight(fontMetrics().height() );
    QVBoxLayout *vbox = new QVBoxLayout( frmProgress );
    vbox->addWidget(m_progress);
    m_progress->hide();

    connect(m_zoomTimer, SIGNAL(timeout()), this, SLOT(updateZoom()));
    connect(m_filterTimer, SIGNAL(timeout()), this, SLOT(runFilterHelper()));

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

KisPreviewWidget::~KisPreviewWidget() { }

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
    m_filterZoom = 1.0;

    KisConfig cfg;
    QString monitorProfileName = cfg.monitorProfile();
    m_profile = KisMetaRegistry::instance()->csRegistry()->getProfileByName(monitorProfileName);

    QRect r = dev->exactBounds();

    m_groupBox->setTitle(i18n("Preview: ") + dev->name());
    m_previewIsDisplayed = true;

    m_zoom = -1.0;
    zoomChanged(double(m_preview->width()) / double(r.width()) );
}

void KisPreviewWidget::updateZoom()
{
    QApplication::setOverrideCursor(KisCursor::waitCursor());    

    if(m_previewIsDisplayed)
    {
        if(m_dirtyPreview)
        {
            QSize r = m_previewDevice->extent().size();
            int w = r.width(), h = r.height();
            int sw = int(ceil(m_zoom * w / m_filterZoom));
            int sh = int(ceil(m_zoom * h / m_filterZoom));
            m_dirtyPreview = false;
            m_scaledPreview = m_previewDevice->convertToQImage(m_profile, 0, 0, w, h);
            m_scaledPreview = m_scaledPreview.scale(sw,sh, QImage::ScaleMax); // Use scale instead of smoothScale for speed up
        }
        m_preview->setImage(m_scaledPreview);
    } else
    {
        if(m_dirtyOriginal)
        {
            QSize r = m_origDevice->extent().size();
            int w = r.width(), h = r.height();
            int sw = int(ceil(m_zoom * w));
            int sh = int(ceil(m_zoom * h));
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
        // Call directly without any pause because there is no scaling
        updateZoom();
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

        if(m_firstZoom) {
            m_firstZoom = false;
            updateZoom();
        } else {
            m_zoomTimer->start(ZOOM_PAUSE, true);
        }
    }
}

void KisPreviewWidget::zoomIn() {
    zoomChanged(m_zoom * ZOOM_FACTOR);
}

void KisPreviewWidget::zoomOut() {
    zoomChanged(m_zoom / ZOOM_FACTOR);
}

void KisPreviewWidget::zoomOneToOne() {
    zoomChanged(1.0);
}

class MyCropVisitor : public KisLayerVisitor {
    const double m_zoom;

    void cropDevice(KisPaintDevice * device) {
        QRect r = device->exactBounds();
        r.setX(int(m_zoom * r.x()) );
        r.setY(int(m_zoom * r.y()) );
        r.setWidth(int(m_zoom * r.width()) );
        r.setHeight(int(m_zoom * r.height()) );
        device->crop(r);
    }

public:
    MyCropVisitor(const double & z) : m_zoom(z) { }
    virtual ~MyCropVisitor() { }

    virtual bool visit(KisPaintLayer *layer) {
        KisPaintDeviceSP device = layer->paintDevice();
        cropDevice(device.data());
        // Make sure we have a tight fit for the selection
        if(device->hasSelection()) {
            cropDevice(device->selection().data());
        }

        return true;
    }
    virtual bool visit(KisGroupLayer *layer) {
        for(KisLayerSP l = layer->firstChild(); l; l = l->nextSibling()) {
            l->accept(*this);
        }
        return true;
    }
    virtual bool visit(KisPartLayer *) { return true; }
    virtual bool visit(KisAdjustmentLayer *) { return true; }
};

void KisPreviewWidget::runFilter(KisFilter * filter, KisFilterConfiguration * config) {
    if(!filter) return;
    if(!config) return;

    m_filter = filter;
    m_config = config;

    if(m_firstFilter) {
        m_firstFilter = false;
        runFilterHelper();
    } else {
        m_filterTimer->start(FILTER_PAUSE, true);
    }
}

void KisPreviewWidget::runFilterHelper() {
    // Copy the image and scale
    Q_ASSERT(m_origDevice->image());
    m_filterZoom = m_zoom;
    // Dont scale more then 1.0 so we don't waste time in preview widget for large scaling.
    if(m_filterZoom > 1.0) {
        m_filterZoom = 1.0;
    }

    m_scaledImage = new KisImage(*m_origDevice->image());
    KisPaintDeviceSP scaledDevice = m_scaledImage->activeDevice();
    Q_ASSERT(scaledDevice);


    KisSelectionSP select;
    if(scaledDevice->hasSelection())
    {
        select = new KisSelection(*scaledDevice->selection());
        scaledDevice->deselect();
        Q_ASSERT(scaledDevice->hasSelection() == false);
    }
    // Scale
    m_scaledImage->setUndoAdapter(NULL);
    KisHermiteFilterStrategy strategy;
    m_scaledImage->scale(m_filterZoom, m_filterZoom, NULL, &strategy);
    // Scale the selection
    if(select)
    {
        KisPaintDeviceSP t = select.data();        
        KisTransformWorker tw(t, m_filterZoom, m_filterZoom, 0.0, 0.0, 0.0, 0, 0, NULL, &strategy);
        tw.run();
        scaledDevice->setSelection(select);
        select->setParentLayer(scaledDevice->parentLayer());
    }

    // Crop by the zoom value instead of cropping by rectangle. It gives better results
    MyCropVisitor v(m_filterZoom);
    m_scaledImage->rootLayer()->accept(v);

    m_previewDevice = new KisPaintDevice(*scaledDevice);

    // Setup the progress display
    m_filter->enableProgress();
    m_progress->setSubject(m_filter, true, true);
    m_filter->setProgressDisplay(m_progress);
    m_filter->process(scaledDevice, m_previewDevice, m_config, scaledDevice->exactBounds());
    m_filter->disableProgress();

    m_dirtyPreview = true;

    if(m_firstZoom) {
        m_firstZoom = false;
        updateZoom();
    } else {
        m_zoomTimer->start(ZOOM_PAUSE, true);
    }
}

#include "kis_previewwidget.moc"
