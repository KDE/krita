/*
 *  SPDX-FileCopyrightText: 2015 Thorsten Zachmann <zachmann@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "filter/kis_color_transformation_configuration.h"

#include <QMutexLocker>
#include <QMap>
#include <QThread>
#include "filter/kis_color_transformation_filter.h"

struct Q_DECL_HIDDEN KisColorTransformationConfiguration::Private {
    Private()
    {}

    ~Private()
    {
        destroyCache();
    }

    void destroyCache()
    {
        QMutexLocker locker(&mutex);
        qDeleteAll(colorTransformation);
        colorTransformation.clear();
    }

    // XXX: Threadlocal storage!!!
    QMap<QThread*, KoColorTransformation*> colorTransformation;
    QMutex mutex;
};

KisColorTransformationConfiguration::KisColorTransformationConfiguration(const QString & name, qint32 version, KisResourcesInterfaceSP resourcesInterface)
    : KisFilterConfiguration(name, version, resourcesInterface)
    , d(new Private())
{
}

KisColorTransformationConfiguration::KisColorTransformationConfiguration(const KisColorTransformationConfiguration &rhs)
    : KisFilterConfiguration(rhs)
    , d(new Private())
{
}

KisColorTransformationConfiguration::~KisColorTransformationConfiguration()
{
    delete d;
}

KisFilterConfigurationSP KisColorTransformationConfiguration::clone() const
{
    return new KisColorTransformationConfiguration(*this);
}

/**
 * Invalidate the cache by default when setProperty is called. This forces
 * regenerating the color transforms also when a property of this object
 * changes, not only when the object is copied
 */
void KisColorTransformationConfiguration::setProperty(const QString &name, const QVariant &value)
{
    KisFilterConfiguration::setProperty(name, value);
    invalidateColorTransformationCache();
}

KoColorTransformation* KisColorTransformationConfiguration::colorTransformation(const KoColorSpace *cs, const KisColorTransformationFilter *filter) const
{
    QMutexLocker locker(&d->mutex);
    KoColorTransformation *transformation = d->colorTransformation.value(QThread::currentThread(), 0);
    if (!transformation) {
        KisFilterConfigurationSP config(clone().data());
        transformation = filter->createTransformation(cs, config);
        d->colorTransformation.insert(QThread::currentThread(), transformation);
    }
    locker.unlock();
    return transformation;
}

void KisColorTransformationConfiguration::invalidateColorTransformationCache()
{
    d->destroyCache();
}
