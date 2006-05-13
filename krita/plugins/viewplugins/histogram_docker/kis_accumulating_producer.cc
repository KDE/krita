/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <QThread>
#include <QApplication>
#include <QEvent>
//Added by qt3to4:
#include <QCustomEvent>

#include "kis_accumulating_producer.h"

static const int EmitCompletedType = QEvent::User + 1;

/**
 * The threaded producer definition in c++ file because this is really an internal affair.
 * Note that since we _know_ that we'll only have a single instance of it running, at most,
 * we don't care too much about locking and synchronization
 **/
class KisAccumulatingHistogramProducer::ThreadedProducer : public QThread {
    KisAccumulatingHistogramProducer* m_source;
    bool m_stop;
protected:
    virtual void run();
public:
    ThreadedProducer(KisAccumulatingHistogramProducer* source)
        : m_source(source), m_stop(false) {}
    void cancel() { m_stop = true; }
};

KisAccumulatingHistogramProducer::KisAccumulatingHistogramProducer(KisCachedHistogramObserver::Producers* source)
    : KisBasicHistogramProducer(
        KisID("ACCHISTO", ""),
        source->at(0)->channels().count(),
        source->at(0)->numberOfBins(),
        0),
      m_source(source)
{
    m_thread = new ThreadedProducer(this);
}

KisAccumulatingHistogramProducer::~KisAccumulatingHistogramProducer() {
    m_thread->cancel();
    m_thread->wait();
    delete m_thread;
}

void KisAccumulatingHistogramProducer::addRegionsToBinAsync() {
    m_thread->cancel();
    m_thread->wait();
    clear();
    m_thread->start();
}

void KisAccumulatingHistogramProducer::ThreadedProducer::run() {
    m_stop = false;

    uint count = m_source->m_source->count(); // Talk about bad naming schemes...
    KisCachedHistogramObserver::Producers* source = m_source->m_source;
    Q3ValueVector<vBins>& bins = m_source->m_bins;
    int channels = m_source->m_channels;
    int nrOfBins = m_source->m_nrOfBins;

    for (uint i = 0; i < count && !m_stop; i++) {
        KisHistogramProducer* p = source->at(i);
        m_source->m_count += p->count();

        for (int j = 0; j < channels && !m_stop; j++) {
            for (int k = 0; k < nrOfBins; k++) {
                bins.at(j).at(k) += p->getBinAt(j, k);
            }
        }
    }

    if (!m_stop) {
        // This function is thread-safe; and it takes ownership of the event
        QApplication::postEvent(m_source, new QCustomEvent(EmitCompletedType));
    }
}

void KisAccumulatingHistogramProducer::customEvent(QCustomEvent* e) {
    if (e->type() == EmitCompletedType) {
        emit completed();
    }
}

#include "kis_accumulating_producer.moc"

