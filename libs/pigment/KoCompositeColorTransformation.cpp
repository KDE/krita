/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoCompositeColorTransformation.h"

#include <QVector>


struct Q_DECL_HIDDEN KoCompositeColorTransformation::Private
{
    ~Private() {
        qDeleteAll(transformations);
    }

    QVector<KoColorTransformation*> transformations;
};


KoCompositeColorTransformation::KoCompositeColorTransformation(Mode mode)
    : m_d(new Private)
{
    Q_ASSERT_X(mode == INPLACE, "KoCompositeColorTransformation", "BUFFERED mode is not implemented yet!");
    Q_UNUSED(mode);
}

KoCompositeColorTransformation::~KoCompositeColorTransformation()
{
}

void KoCompositeColorTransformation::appendTransform(KoColorTransformation *transform)
{
    if (transform) {
        m_d->transformations.append(transform);
    }
}

void KoCompositeColorTransformation::transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
{
    QVector<KoColorTransformation*>::const_iterator begin = m_d->transformations.constBegin();
    QVector<KoColorTransformation*>::const_iterator it = begin;
    QVector<KoColorTransformation*>::const_iterator end = m_d->transformations.constEnd();

    for (; it != end; ++it) {
        if (it == begin) {
            (*it)->transform(src, dst, nPixels);
        } else {
            (*it)->transform(dst, dst, nPixels);
        }
    }
}

KoColorTransformation* KoCompositeColorTransformation::createOptimizedCompositeTransform(const QVector<KoColorTransformation*> transforms)
{
    KoColorTransformation *finalTransform = 0;

    int numValidTransforms = 0;
    foreach (KoColorTransformation *t, transforms) {
        numValidTransforms += bool(t);
    }

    if (numValidTransforms > 1) {
        KoCompositeColorTransformation *compositeTransform =
            new KoCompositeColorTransformation(
                KoCompositeColorTransformation::INPLACE);

        foreach (KoColorTransformation *t, transforms) {
            if (t) {
                compositeTransform->appendTransform(t);
            }
        }

        finalTransform = compositeTransform;

    } else if (numValidTransforms == 1) {
        foreach (KoColorTransformation *t, transforms) {
            if (t) {
                finalTransform = t;
                break;
            }
        }
    }

    return finalTransform;
}
