/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_suspend_projection_updates_stroke_strategy.h"

#include <kis_image.h>
#include <krita_utils.h>
#include <kis_projection_updates_filter.h>


inline uint qHash(const QRect &rc) {
    return rc.x() +
        (rc.y() << 16) +
        (rc.width() << 8) +
        (rc.height() << 24);
}

struct KisSuspendProjectionUpdatesStrokeStrategy::Private
{
    KisImageWSP image;
    bool suspend;

    class SuspendLod0Updates : public KisProjectionUpdatesFilter
    {

        typedef QHash<KisNodeSP, QVector<QRect> > RectsHash;
    public:
        struct Request {
            Request() {}
            Request(KisNodeSP _node, QRect _rect) : node(_node), rect(_rect) {}
            KisNodeSP node;
            QRect rect;
        };
    public:
        SuspendLod0Updates()
        {
        }

        bool filter(KisImage *image, KisNode *node, const QRect &rect) {
            if (image->currentLevelOfDetail() > 0) return false;

            QMutexLocker l(&m_mutex);
            m_requestsHash[KisNodeSP(node)].append(rect);
            return true;
        }

        static inline QRect alignRect(const QRect &rc, const int step) {
            static const int decstep = step - 1;
            static const int invstep = ~decstep;

            int x0, y0, x1, y1;
            rc.getCoords(&x0, &y0, &x1, &y1);

            x0 &= invstep;
            y0 &= invstep;
            x1 |= decstep;
            y1 |= decstep;

            QRect result;
            result.setCoords(x0, y0, x1, y1);
            return result;
        }

        void notifyUpdates(KisNodeGraphListener *listener) {
            RectsHash::const_iterator it = m_requestsHash.constBegin();
            RectsHash::const_iterator end = m_requestsHash.constEnd();

            const int step = 64;

            for (; it != end; ++it) {
                KisNodeSP node = it.key();
                const QVector<QRect> &rects = it.value();

                QVector<QRect>::const_iterator it = rects.constBegin();
                QVector<QRect>::const_iterator end = rects.constEnd();

                QRegion region;

                for (; it != end; ++it) {
                    region += alignRect(*it, step);
                }

                foreach(const QRect &rc, region.rects()) {
                    // FIXME: constness: port rPU to SP
                    listener->requestProjectionUpdate(const_cast<KisNode*>(node.data()), rc);

                }
            }
        }

    private:
        RectsHash m_requestsHash;
        QMutex m_mutex;
    };
};

KisSuspendProjectionUpdatesStrokeStrategy::KisSuspendProjectionUpdatesStrokeStrategy(KisImageWSP image, bool suspend)
    : KisSimpleStrokeStrategy(suspend ? "suspend_stroke_strategy" : "resume_stroke_strategy"),
      m_d(new Private)
{
    m_d->image = image;
    m_d->suspend = suspend;

    // TODO: exclusive
    enableJob(JOB_FINISH, true);
    enableJob(JOB_CANCEL, true);
}

KisSuspendProjectionUpdatesStrokeStrategy::~KisSuspendProjectionUpdatesStrokeStrategy()
{
}

void KisSuspendProjectionUpdatesStrokeStrategy::finishStrokeCallback()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->image);

    if (m_d->suspend) {
        m_d->image->setProjectionUpdatesFilter(
            KisProjectionUpdatesFilterSP(new Private::SuspendLod0Updates()));
    } else {
        KisProjectionUpdatesFilterSP filter =
            m_d->image->projectionUpdatesFilter();

        if (!filter) return;

        Private::SuspendLod0Updates *localFilter =
            dynamic_cast<Private::SuspendLod0Updates*>(filter.data());

        KIS_ASSERT_RECOVER_NOOP(localFilter);

        if (localFilter) {
            m_d->image->setProjectionUpdatesFilter(KisProjectionUpdatesFilterSP());
            localFilter->notifyUpdates(m_d->image.data());
        }
    }
}

void KisSuspendProjectionUpdatesStrokeStrategy::cancelStrokeCallback()
{
    finishStrokeCallback();

    if (!m_d->suspend) {
        // FIXME: optimize
        m_d->image->refreshGraphAsync();
    }
}
