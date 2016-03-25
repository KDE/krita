/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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
    Q_UNUSED(mode)
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
