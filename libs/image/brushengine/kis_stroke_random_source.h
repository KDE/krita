/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_STROKE_RANDOM_SOURCE_H
#define __KIS_STROKE_RANDOM_SOURCE_H

#include <QScopedPointer>

#include "kritaimage_export.h"
#include "kis_random_source.h"
#include "KisPerStrokeRandomSource.h"


/**
 * A helper class to handle multiple KisRandomSource objects in a
 * stroke strategies. It creates two identical random sources in the
 * beginning of the stroke, so, when copied through copy-ctor and set
 * another level of detail starts returning the same sequence of
 * numbers as was returned for the first stroke.
 */
class KRITAIMAGE_EXPORT  KisStrokeRandomSource
{
public:
    KisStrokeRandomSource();
    KisStrokeRandomSource(const KisStrokeRandomSource &rhs);
    KisStrokeRandomSource& operator=(const KisStrokeRandomSource &rhs);

    ~KisStrokeRandomSource();

    KisRandomSourceSP source() const;
    KisPerStrokeRandomSourceSP perStrokeSource() const;

    int levelOfDetail() const;
    void setLevelOfDetail(int value);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_STROKE_RANDOM_SOURCE_H */
