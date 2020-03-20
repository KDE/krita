/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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
#include "KisGlobalResourcesInterface.h"

#include <QGlobalStatic>
#include <KisResourceModelProvider.h>
#include <KisResourceModel.h>

#include <kis_debug.h>

#include <QBasicMutex>


namespace {
class GlobalResourcesSource : public KisResourcesInterface::ResourceSourceAdapter
{
public:
    GlobalResourcesSource(KisResourceModel *model) : m_model(model) {}

    KoResourceSP resourceForFilename(const QString& filename) const override {
        return m_model->resourceForFilename(filename);
    }
    KoResourceSP resourceForName(const QString& name) const override {
        return m_model->resourceForName(name);
    }
    KoResourceSP resourceForMD5(const QByteArray& md5) const override {
        return m_model->resourceForMD5(md5);
    }
    KoResourceSP fallbackResource() const override {
        return m_model->rowCount() > 0 ? m_model->resourceForIndex(m_model->index(0, 0)) : KoResourceSP();
    }

private:
    KisResourceModel *m_model;
};
}

KisResourcesInterfaceSP KisGlobalResourcesInterface::instance()
{
    // TODO: implement general macro like Q_GLOBAL_STATIC()

    static QBasicAtomicInt guard = Q_BASIC_ATOMIC_INITIALIZER(QtGlobalStatic::Uninitialized);
    static KisResourcesInterfaceSP d;
    static QBasicMutex mutex;
    int x = guard.loadAcquire();
    if (Q_UNLIKELY(x >= QtGlobalStatic::Uninitialized)) {
        QMutexLocker locker(&mutex);
        if (guard.load() == QtGlobalStatic::Uninitialized) {
            d.reset(new KisGlobalResourcesInterface());
            static struct Cleanup {
                ~Cleanup() {
                    d.reset();
                    guard.store(QtGlobalStatic::Destroyed);
                }
            } cleanup;
            guard.storeRelease(QtGlobalStatic::Initialized);
        }
    }

    return d;
}

KisResourcesInterface::ResourceSourceAdapter *KisGlobalResourcesInterface::createSourceImpl(const QString &type) const
{
    KisResourcesInterface::ResourceSourceAdapter *source =
        new GlobalResourcesSource(KisResourceModelProvider::resourceModel(type));

    KIS_ASSERT(source);
    return source;
}
