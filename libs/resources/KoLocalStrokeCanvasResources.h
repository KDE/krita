/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOLOCALSTROKECANVASRESOURCES_H
#define KOLOCALSTROKECANVASRESOURCES_H

#include "KoCanvasResourcesInterface.h"

#include <QScopedPointer>
#include <QSharedPointer>

#include <kritaresources_export.h>

class KRITARESOURCES_EXPORT KoLocalStrokeCanvasResources : public KoCanvasResourcesInterface
{
public:
    KoLocalStrokeCanvasResources();
    KoLocalStrokeCanvasResources(const KoLocalStrokeCanvasResources &rhs);
    KoLocalStrokeCanvasResources& operator=(const KoLocalStrokeCanvasResources &rhs);
    ~KoLocalStrokeCanvasResources();

    QVariant resource(int key) const override;
    void storeResource(int key, const QVariant &resource);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

using KoLocalStrokeCanvasResourcesSP = QSharedPointer<KoLocalStrokeCanvasResources>;

#endif // KOLOCALSTROKECANVASRESOURCES_H
