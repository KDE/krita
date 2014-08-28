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

#ifndef __KIS_PROJECTION_UPDATES_FILTER_H
#define __KIS_PROJECTION_UPDATES_FILTER_H

#include <QSharedPointer>

class KisImage;
class KisNode;
class QRect;

class KisProjectionUpdatesFilter
{
public:
    virtual ~KisProjectionUpdatesFilter();

    /**
     * \return true if an update should be dropped by the image
     */
    virtual bool filter(KisImage *image, KisNode *node, const QRect& rect) = 0;
};



/**
 * A dummy filter implementation that eats all the updates
 */
class KisDropAllProjectionUpdatesFilter : public KisProjectionUpdatesFilter
{
public:
    bool filter(KisImage *image, KisNode *node, const QRect& rect);
};

#endif /* __KIS_PROJECTION_UPDATES_FILTER_H */
