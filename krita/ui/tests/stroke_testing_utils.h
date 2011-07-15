/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __STROKE_TESTING_UTILS_H
#define __STROKE_TESTING_UTILS_H

#include <QString>
#include <KoResourceManager.h>
#include "kis_node.h"
#include "kis_types.h"
#include "kis_stroke_strategy.h"
#include "kis_resources_snapshot.h"


class KisUndoAdapter;


namespace utils {

    KisImageSP createImage(KisUndoAdapter *undoAdapter, const QSize &imageSize);
    KoResourceManager* createResourceManager(KisImageWSP image,
                                             KisNodeSP node = 0,
                                             const QString &presetFileName = "autobrush_300px.kpp");

    class StrokeTester
    {
    public:
        StrokeTester(const QString &name, const QSize &imageSize);
        virtual ~StrokeTester();

        void test();
        void benchmark();

    protected:
        KisStrokeId strokeId() {
            return m_strokeId;
        }

        virtual KisStrokeStrategy* createStroke(bool indirectPainting,
                                                KisResourcesSnapshotSP resources,
                                                KisPainter *painter) = 0;

        virtual void addPaintingJobs(KisImageWSP image,
                                     KisResourcesSnapshotSP resources,
                                     KisPainter *painter) = 0;

    private:
        void testOneStroke(bool cancelled, bool indirectPainting);

        QImage doStroke(bool cancelled, bool indirectPainting, bool needQImage = true);
        QString formatFilename(const QString &baseName,  bool cancelled, bool indirectPainting);

    private:
        KisStrokeId m_strokeId;
        QString m_name;
        QSize m_imageSize;
    };
}

#endif /* __STROKE_TESTING_UTILS_H */
