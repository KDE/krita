/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_RANDOM_SOURCE_H
#define __KIS_RANDOM_SOURCE_H

#include <QScopedPointer>
#include "kis_shared.h"
#include "kis_shared_ptr.h"

#include "kritaimage_export.h"

/**
 * KisRandomSource is a special object that wraps around random number
 * generation routines.
 *
 * It has the following properties:
 *
 * 1) Two KisRandomSource objects will generate exactly the same sequences of
 *    numbers if created with the same seed.
 *
 * 2) After copy-construction or assignment the two objects will
 *    continue to generate exactly the same numbers. Imagine like the
 *    history got forked.
 *
 * 3) Copying of a KisRandomSource object is fast. It uses Tauss88
 *    algorithm to achieve this.
 */
class KRITAIMAGE_EXPORT KisRandomSource : public KisShared
{
public:
    KisRandomSource();
    KisRandomSource(int seed);
    KisRandomSource(const KisRandomSource &rhs);
    KisRandomSource& operator=(const KisRandomSource &rhs);

    ~KisRandomSource();

    /**
     * Generates a random number in a range from min() to max()
     */
    qint64 generate() const;

    /**
     * Generates a random number in a range from \p min to \p max
     */
    int generate(int min, int max) const;

    /**
     * Generates a random number in a closed range [0; 1.0]
     */
    qreal generateNormalized() const;

    /**
     * Generates a number from the Gaussian distribution
     */
    qreal generateGaussian(qreal mean, qreal sigma) const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

class KisRandomSource;
typedef KisSharedPtr<KisRandomSource> KisRandomSourceSP;
typedef KisWeakSharedPtr<KisRandomSource> KisRandomSourceWSP;

#endif /* __KIS_RANDOM_SOURCE_H */
