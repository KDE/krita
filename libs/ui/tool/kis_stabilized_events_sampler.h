/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_STABILIZED_EVENTS_SAMPLER_H
#define __KIS_STABILIZED_EVENTS_SAMPLER_H

#include <QScopedPointer>

#include <boost/iterator/iterator_facade.hpp>

#include "kritaui_export.h"

class KisPaintInformation;
#include <kis_paint_information.h>


class KRITAUI_EXPORT KisStabilizedEventsSampler
{
public:
    KisStabilizedEventsSampler(int sampleTime = 1);
    ~KisStabilizedEventsSampler();

    void clear();
    void addEvent(const KisPaintInformation &pi);
    void addFinishingEvent(int numSamples);

public:
    class KRITAUI_EXPORT iterator :
        public boost::iterator_facade <iterator,
                                       KisPaintInformation const,
                                       boost::forward_traversal_tag >
    {
    public:
        iterator()
            : m_sampler(0),
              m_index(0),
              m_alpha(0) {}

        iterator(const KisStabilizedEventsSampler* sampler, int index, qreal alpha)
            : m_sampler(sampler),
              m_index(index),
              m_alpha(alpha) {}

    private:
        friend class boost::iterator_core_access;

        void increment() {
            m_index++;
        }

        bool equal(iterator const& other) const {
            return m_index == other.m_index &&
                m_sampler == other.m_sampler;
        }

        const KisPaintInformation& dereference() const;

    private:
        const KisStabilizedEventsSampler* m_sampler;
        int m_index;
        qreal m_alpha;
    };

    std::pair<iterator, iterator> range() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif /* __KIS_STABILIZED_EVENTS_SAMPLER_H */
