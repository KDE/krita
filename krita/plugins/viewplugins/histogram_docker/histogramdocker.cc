/*
 * This file is part of the KDE project
 *
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kopalettemanager.h>

#include "kis_meta_registry.h"
#include "kis_doc.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view.h"

#include "kis_basic_histogram_producers.h"
#include "KoColorSpaceFactoryRegistry.h"

#include "histogramdocker.h"
#include "kis_imagerasteredcache.h"
#include "kis_accumulating_producer.h"
#include "KoID.h"

typedef KGenericFactory<KritaHistogramDocker> KritaHistogramDockerFactory;
K_EXPORT_COMPONENT_FACTORY( kritahistogramdocker, KritaHistogramDockerFactory( "krita" ) )

KritaHistogramDocker::KritaHistogramDocker(QObject *parent, const QStringList&)
        : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView") ) {
        m_view = dynamic_cast<KisView*>(parent);

        setInstance(KritaHistogramDockerFactory::instance());
        
setXMLFile(KStandardDirs::locate("data","kritaplugins/kritahistogramdocker.rc"), 
true);

        KisImageSP img = m_view->canvasSubject()->currentImg();
        if (!img) {
            m_cache = 0;
            return;
        }

        m_hview = 0; // producerChanged wants to setCurrentChannels, prevent that here
        m_cache = 0; // we try to delete it in producerChanged
        colorSpaceChanged(img->colorSpace()); // calls producerChanged(0)


        m_hview = new KisHistogramView(m_view);
        m_hview->setHistogram(m_histogram);
        m_hview->setColor(true);
        m_hview->setCurrentChannels(KisHistogramProducerSP(m_producer), m_producer->channels());
        m_hview->setFixedSize(256, 100); // XXX if not it keeps expanding
        m_hview->setWindowTitle(i18n("Histogram"));


        connect(m_hview, SIGNAL(rightClicked(const QPoint&)),
                this, SLOT(popupMenu(const QPoint&)));
        connect(m_cache, SIGNAL(cacheUpdated()),
                new HistogramDockerUpdater(this, m_histogram, m_hview, m_producer), SLOT(updated()));
        connect(&m_popup, SIGNAL(triggered(QAction *)),
                this, SLOT(producerChanged(QAction *)));
        connect(img.data(), SIGNAL(sigColorSpaceChanged(KoColorSpace*)),
                this, SLOT(colorSpaceChanged(KoColorSpace*))); // No need to force updates here

        // Add it to the control palette
        m_view->canvasSubject()->paletteManager()->addWidget(
            m_hview, "histodocker", krita::CONTROL_PALETTE);
    } else {
        m_cache = 0;
    }
}

KritaHistogramDocker::~KritaHistogramDocker()
{
    uint count = m_producers . count();
    for (uint i = 0; i < count; i++) {
        delete m_producers . at(i);
    }

    if (m_cache)
        m_cache->deleteLater();
}

void KritaHistogramDocker::producerChanged(QAction *action)
{
    int pos = m_popup.actions().indexOf(action);

    if (m_cache)
        m_cache->deleteLater();
    m_cache = 0;

    if (m_currentProducerPos < m_popup.actions().count())
        m_popup.actions().at(m_currentProducerPos)->setChecked(false);
    m_currentProducerPos = pos;
    m_popup.actions().at(m_currentProducerPos)->setChecked(true);

    uint count = m_producers . count();
    for (uint i = 0; i < count; i++) {
        delete m_producers . at(i);
    }
    m_producers.clear();

    QList<KoID> keys = KisHistogramProducerFactoryRegistry::instance() ->
            listKeysCompatibleWith(m_cs);

    m_factory = KisHistogramProducerFactoryRegistry::instance()->get(keys.at(pos));

    KisCachedHistogramObserver observer(&m_producers, m_factory, 0, 0, 0, 0, false);

    // We can reference observer because it will be only used as a factory to create new
    // instances
    m_cache = new KisImageRasteredCache(m_view, &observer);

    m_producer = new KisAccumulatingHistogramProducer(&m_producers);

    // use dummy layer as a source; we are not going to actually use or need it
    // All of these are SP, no need to delete them afterwards
    m_histogram = new KisHistogram( KisPaintDeviceSP(new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getAlpha8(), "dummy histogram")),
                                    KisHistogramProducerSP(m_producer), LOGARITHMIC);

    if (m_hview) {
        m_hview->setHistogram(m_histogram);
        m_hview->setColor(true);
        m_hview->setCurrentChannels(KisHistogramProducerSP(m_producer), m_producer->channels());

        connect(m_cache, SIGNAL(cacheUpdated()),
                new HistogramDockerUpdater(this, m_histogram, m_hview, m_producer), SLOT(updated()));
    }
}

void KritaHistogramDocker::popupMenu(const QPoint& pos)
{
    m_popup.popup(pos, m_popup.actions().at(m_currentProducerPos));
}

void KritaHistogramDocker::colorSpaceChanged(KoColorSpace* cs)
{
    m_cs = cs;

    QList<KoID> keys = KisHistogramProducerFactoryRegistry::instance() ->
            listKeysCompatibleWith(m_cs);

    m_popup.clear();
    m_currentProducerPos = 0;

    for (int i = 0; i < keys.count(); i++) {
        KoID id(keys.at(i));
        m_popup.addAction(id.name());
    }

    producerChanged(m_popup.actions().at(0));
}

HistogramDockerUpdater::HistogramDockerUpdater(QObject* /*parent*/, KisHistogramSP h, KisHistogramView* v,
                                               KisAccumulatingHistogramProducer* p)
    : m_histogram(h), m_view(v), m_producer(p)
{
    connect(p, SIGNAL(completed()), this, SLOT(completed()));
}

void HistogramDockerUpdater::updated() {
    // We don't [!] do m_histogram->updateHistogram();, because that will try to compute
    // the histogram synchronously, while we want it asynchronously.
    m_producer->addRegionsToBinAsync();
}

void HistogramDockerUpdater::completed() {
    m_histogram->computeHistogram();
    m_view->updateHistogram();
}

#include "histogramdocker.moc"
