/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISLOCALSTROKERESOURCES_H
#define KISLOCALSTROKERESOURCES_H

#include <KisResourcesInterface.h>

class KisLocalStrokeResourcesPrivate;


/**
 * @brief a KisResourcesInterface-like resources storage for preloaded resources
 *
 * KisLocalStrokeResources stores preloaded resources and dispatches them
 * to the consumers as a resources source.
 *
 * It is used by the strokes to avoid accessing global resource storage
 * from non-gui threads.
 */
class KRITARESOURCES_EXPORT KisLocalStrokeResources : public KisResourcesInterface
{
public:
    KisLocalStrokeResources(const QList<KoResourceSP> &localResources);

protected:
    ResourceSourceAdapter* createSourceImpl(const QString &type) const override;

private:
    Q_DECLARE_PRIVATE(KisLocalStrokeResources);
};

#endif // KISLOCALSTROKERESOURCES_H
