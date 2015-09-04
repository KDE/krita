/* This file is part of the Calligra libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#include "KisFilterChainLink.h"
#include <QMetaMethod>
#include <QPluginLoader>
#include <QTemporaryFile>
#include <kmimetype.h>
#include <kdebug.h>
#include "KisFilterEntry.h"
#include "KisImportExportManager.h"
#include "KoProgressUpdater.h"
#include "KoUpdater.h"

namespace
{
    const char *const SIGNAL_PREFIX = "commSignal";
    const int SIGNAL_PREFIX_LEN = 10;
    const char *const SLOT_PREFIX = "commSlot";
    const int SLOT_PREFIX_LEN = 8;

    KoUpdater *createUpdater(KisFilterChain *chain)
    {
        QPointer<KoUpdater> updater = 0;
        Q_ASSERT(chain);
        Q_ASSERT(chain->manager());
        KoProgressUpdater *pu = chain->manager()->progressUpdater();
        if (pu) {
            updater = pu->startSubtask(1, "filter");
            updater->setProgress(0);
        }

        return updater;
    }
}

namespace CalligraFilter {

    ChainLink::ChainLink(KisFilterChain *chain, KisFilterEntry::Ptr filterEntry,
                         const QByteArray& from, const QByteArray& to)
        : m_chain(chain)
        , m_filterEntry(filterEntry)
        , m_from(from)
        , m_to(to)
        , m_filter(0)
        , m_updater(createUpdater(chain))
    {
    }

    ChainLink::~ChainLink() {
    }

    KisImportExportFilter::ConversionStatus ChainLink::invokeFilter(const ChainLink *const parentChainLink)
    {
        if (!m_filterEntry) {
            kError(30500) << "This filter entry is null. Strange stuff going on." << endl;
            return KisImportExportFilter::FilterEntryNull;
        }

        m_filter = m_filterEntry->createFilter(m_chain);

        if (!m_filter) {
            kError(30500) << "Couldn't create the filter." << endl;
            return KisImportExportFilter::FilterCreationError;
        }

        if (m_updater) {
            // if there is an updater, use that for progress reporting
            m_filter->setUpdater(m_updater);
        }

        if (parentChainLink) {
            setupCommunication(parentChainLink->m_filter);
        }

        KisImportExportFilter::ConversionStatus status = m_filter->convert(m_from, m_to);
        delete m_filter;
        m_filter = 0;
        if (m_updater) {
            m_updater->setProgress(100);
        }
        return status;
    }

    void ChainLink::dump() const
    {
        kDebug(30500) << "   Link:" << m_filterEntry->loader()->fileName();
    }

    void ChainLink::setupCommunication(const KisImportExportFilter *const parentFilter) const
    {
        if (!parentFilter)
            return;

        const QMetaObject *const parent = parentFilter->metaObject();
        const QMetaObject *const child = m_filter->metaObject();
        if (!parent || !child)
            return;

        setupConnections(parentFilter, m_filter);
        setupConnections(m_filter, parentFilter);
    }

    void ChainLink::setupConnections(const KisImportExportFilter *sender, const KisImportExportFilter *receiver) const
    {
        const QMetaObject * const parent = sender->metaObject();
        const QMetaObject * const child = receiver->metaObject();
        if (!parent || !child)
            return;

        int senderMethodCount = parent->methodCount();
        for (int i = 0; i < senderMethodCount; ++i) {
            QMetaMethod signal = parent->method(i);
            if (signal.methodType() != QMetaMethod::Signal)
                continue;
            // ### untested (QMetaMethod::signature())
            if (strncmp(signal.methodSignature(), SIGNAL_PREFIX, SIGNAL_PREFIX_LEN) == 0) {
                int receiverMethodCount = child->methodCount();
                for (int j = 0; j < receiverMethodCount; ++j) {
                    QMetaMethod slot = child->method(j);
                    if (slot.methodType() != QMetaMethod::Slot)
                        continue;
                    if (strncmp(slot.methodSignature(), SLOT_PREFIX, SLOT_PREFIX_LEN) == 0) {
                        if (strcmp(signal.methodSignature().constData() + SIGNAL_PREFIX_LEN,
                                   slot.methodSignature().constData() + SLOT_PREFIX_LEN) == 0) {
                            QByteArray signalString;
                            signalString.setNum(QSIGNAL_CODE);
                            signalString += signal.methodSignature();
                            QByteArray slotString;
                            slotString.setNum(QSLOT_CODE);
                            slotString += slot.methodSignature();
                            QObject::connect(sender, signalString, receiver, slotString);
                        }
                    }
                }
            }
        }
    }

}
