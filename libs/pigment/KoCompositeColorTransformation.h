/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KO_COMPOSITE_COLOR_TRANSFORMATION_H
#define __KO_COMPOSITE_COLOR_TRANSFORMATION_H

#include "KoColorTransformation.h"

#include <QScopedPointer>


/**
 * A class for storing a composite color transformation. All the
 * transformations added with appendTransform() are applied
 * sequentially to the process pixels.
 *
 * \p mode defines how the buffers are used while processing.
 *
 * When using INPLACE mode all transformations but the first one do
 * the conversion in place, that is in \p dst buffer. That is \p dst
 * is written at least N - 1 times, where N is the number of embedded
 * transformations.
 *
 * In BUFFERED mode all the transformations are called with distinct
 * src and dst buffers, which are created in temporary memory owned by
 * KoCompositeColorTransformation. Please note that this mode IS NOT
 * IMPLEMENTED YET!
 */
class KRITAPIGMENT_EXPORT KoCompositeColorTransformation : public KoColorTransformation
{
public:
    enum Mode {
        INPLACE = 0, /// transform pixels in place (in 'dst' buffer)
        BUFFERED /// transform using a temporary buffer (not implemented yet)
    };

public:
    explicit KoCompositeColorTransformation(Mode mode);
    ~KoCompositeColorTransformation() override;

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override;

    /**
     * Append a transform to a composite. If \p transform is null,
     * nothing happens.
     */
    void appendTransform(KoColorTransformation *transform);

    /**
     * Convenience method that checks if the transformations in \p
     * transforms are not null and adds existent ones only. If there
     * is only one non-null transform, it is returned directly to
     * avoid extra virtual calls added by KoCompositeColorTransformation.
     */
    static KoColorTransformation* createOptimizedCompositeTransform(const QVector<KoColorTransformation*> transforms);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KO_COMPOSITE_COLOR_TRANSFORMATION_H */
