/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisGlobalResourcesInterface.h"

#include <QGlobalStatic>
#include <KisResourceModel.h>
#include <KisResourceModelProvider.h>

#include <kis_debug.h>

#include <QBasicMutex>


namespace {
class GlobalResourcesSource : public KisResourcesInterface::ResourceSourceAdapter
{
public:
    GlobalResourcesSource(const QString &resourceType, KisAllResourcesModel *model)
        : KisResourcesInterface::ResourceSourceAdapter(resourceType)
        , m_model(model)
    {}

    ~GlobalResourcesSource() override
    {
    }
protected:
    QVector<KoResourceSP> resourcesForFilename(const QString &filename) const override {
        QVector<KoResourceSP> res = m_model->resourcesForFilename(filename);
        return res;
    }

    QVector<KoResourceSP> resourcesForName(const QString &name) const override {
        QVector<KoResourceSP> res = m_model->resourcesForName(name);
        return res;
    }

    QVector<KoResourceSP> resourcesForMD5(const QString &md5) const override {
        QVector<KoResourceSP> res = m_model->resourcesForMD5(md5);
        return res;
    }
public:
    KoResourceSP fallbackResource() const override {
        return m_model->rowCount() > 0 ? m_model->resourceForIndex(m_model->index(0, 0)) : KoResourceSP();
    }

private:
    KisAllResourcesModel *m_model;
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
        new GlobalResourcesSource(type, KisResourceModelProvider::resourceModel(type));

    KIS_ASSERT(source);
    return source;
}
