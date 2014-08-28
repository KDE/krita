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
#include "kis_projection_updates_filter.h"

struct KisSuspendProjectionUpdatesStrokeStrategy::Private
{
    KisImageWSP image;
    bool suspend;

    class SuspendLod0Updates : public KisProjectionUpdatesFilter
    {
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
            m_requests.append(Request(node, rect));
            return true;
        }

        const QVector<Request>& requests() const {
            return m_requests;
        }

    private:
        QVector<Request> m_requests;
        QMutex m_mutex;
    };
};

KisSuspendProjectionUpdatesStrokeStrategy::KisSuspendProjectionUpdatesStrokeStrategy(KisImageWSP image, bool suspend)
    : m_d(new Private)
{
    m_d->image = image;
    m_d->suspend = suspend;

    // TODO: exclusive
    enableJob(JOB_INIT, true);
}

KisSuspendProjectionUpdatesStrokeStrategy::~KisSuspendProjectionUpdatesStrokeStrategy()
{
}

void KisSuspendProjectionUpdatesStrokeStrategy::initStrokeCallback()
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

            foreach (const Private::SuspendLod0Updates::Request &req, localFilter->requests()) {
                // FIXME: constness: port rPU to SP
                m_d->image->requestProjectionUpdate(const_cast<KisNode*>(req.node.data()), req.rect);
            }
        }
    }
}
