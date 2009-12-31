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

#include "histogramdock.h"

#include <QDockWidget>
#include <QLabel>

#include <kis_debug.h>

#include "KoBasicHistogramProducers.h"
#include "KoColorSpaceRegistry.h"
#include "KoID.h"

#include <kis_global.h>
#include <kis_types.h>
#include <kis_image.h>
#include <kis_histogram_view.h>
#include <kis_canvas2.h>
#include <kis_view2.h>

#include "kis_imagerasteredcache.h"
#include "kis_accumulating_producer.h"
#include "histogram_updater.h"

KisHistogramDocker::KisHistogramDocker()
    : QDockWidget(i18n("Histogram"))
    , m_canvas(0)
    , m_image(0)
    , m_factory(0)
    , m_producer(0)
    , m_cs(0)
    , m_hview(0)
    , m_cache(0)
    , m_histogram(0)
{
}

KisHistogramDocker::~KisHistogramDocker()
{
    uint count = m_producers . count();
    for (uint i = 0; i < count; i++) {
        delete m_producers . at(i);
    }
    m_producers.clear();
    if (m_cache)
        m_cache->deleteLater();

    delete m_producer;
}

void KisHistogramDocker::setCanvas(KoCanvasBase* canvas)
{
    disconnect();
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    if (m_canvas) {
        connect(m_canvas, SIGNAL(imageChanged(KisImageWSP)), SLOT(setImage(KisImageWSP)));
    }
}

void KisHistogramDocker::setImage(KisImageWSP image)
{
    if (!image) return;

    Q_ASSERT(image);

    m_image = image;

    if (m_canvas && m_canvas->view()) {


        m_hview = 0; // producerChanged wants to setCurrentChannels, prevent that here
        m_cache = 0; // we try to delete it in producerChanged
        colorSpaceChanged(m_image->colorSpace()); // calls producerChanged(0)

        if (m_histogram) {
            m_hview = new KisHistogramView(this);
            m_hview->setHistogram(m_histogram);
            m_hview->setColor(true);

            // At the time we called colorSpaceChanged m_hview was not yet constructed, so producerChanged didn't call this
            setChannels();

            m_hview->setFixedSize(256, 100); // XXX if not it keeps expanding
            m_hview->setWindowTitle(i18n("Histogram"));

            connect(m_hview, SIGNAL(rightClicked(const QPoint&)), SLOT(popupMenu(const QPoint&)));
            connect(m_cache, SIGNAL(cacheUpdated()), new HistogramDockerUpdater(this, m_histogram, m_hview, m_producer), SLOT(updated()));
            connect(&m_popup, SIGNAL(triggered(QAction *)), SLOT(producerChanged(QAction *)));
            connect(m_canvas->view(), SIGNAL(sigLoadingFinished()), SLOT(reset()));
            connect(m_image.data(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), SLOT(colorSpaceChanged(const KoColorSpace*))); // No need to force updates here
            m_cache->setDocker(this);
            m_cache->setImage(m_image);

            setWidget(m_hview);
        }
        else {
            QLabel* l = new QLabel(i18n("Histograms are not supported for images in the %1 colorspace.").arg(m_cs->name()),
                                   m_canvas->view());
            l->setWordWrap(true);
            l->setMargin(4);
            setWidget(l);
        }

    } else {
        delete m_cache;
        m_cache = 0;
    }

}

void KisHistogramDocker::setChannels()
{
    m_hview->setHistogram(m_histogram);
    m_hview->setColor(true);
    QList<KoChannelInfo *> channels;
    // Only display color channels
    for (int i = 0; i < m_producer->channels().count(); i++) {
        if (m_producer->channels().at(i)->channelType() == KoChannelInfo::COLOR) {
            channels.append(m_producer->channels().at(i));
        }
    }
    m_hview->setCurrentChannels(KoHistogramProducerSP(m_producer), channels);
}

void KisHistogramDocker::producerChanged(QAction *action)
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

    QList<QString> keys = KoHistogramProducerFactoryRegistry::instance()->keysCompatibleWith(m_cs);

    m_factory = KoHistogramProducerFactoryRegistry::instance()->get(keys.at(pos));

    KisCachedHistogramObserver observer(&m_producers, m_factory, 0, 0, 0, 0, false);

    // We can reference observer because it will be only used as a factory to create new
    // instances
    m_cache = new KisImageRasteredCache(&observer);
    m_producer = new KisAccumulatingHistogramProducer(&m_producers);

    // use dummy layer as a source; we are not going to actually use or need it
    // All of these are SP, no need to delete them afterwards
    m_histogram = new KisHistogram(new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8()),
                                   KoHistogramProducerSP(m_producer), LOGARITHMIC);

    kDebug() << "created histogram " << m_histogram;

    if (m_hview) {
        setChannels();
        connect(m_cache, SIGNAL(cacheUpdated()),
                new HistogramDockerUpdater(this, m_histogram, m_hview, m_producer), SLOT(updated()));
    }
}

void KisHistogramDocker::popupMenu(const QPoint& pos)
{
    m_popup.popup(pos, m_popup.actions().at(m_currentProducerPos));
}

void KisHistogramDocker::colorSpaceChanged(const KoColorSpace* cs)
{
    kDebug() << cs;
    m_cs = cs;

    QList<QString> keys = KoHistogramProducerFactoryRegistry::instance()->keysCompatibleWith(m_cs);

    m_popup.clear();
    m_currentProducerPos = 0;

    foreach (const QString &key, keys) {
        m_popup.addAction(KoHistogramProducerFactoryRegistry::instance()->get(key)->name());
    }

    if (m_popup.actions().size() > 0) {
        producerChanged(m_popup.actions().at(0));
    }
}

void KisHistogramDocker::reset()
{
    kDebug() << m_image << m_image.isValid();
    if (m_image && m_image.isValid()) {
        colorSpaceChanged(m_image->colorSpace());
    }
}

#include "histogramdock.moc"
