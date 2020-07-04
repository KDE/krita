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
