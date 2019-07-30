/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_CROP_SAVED_EXTRA_DATA_H
#define __KIS_CROP_SAVED_EXTRA_DATA_H

#include <QRect>

#include "kundo2commandextradata.h"
#include "kis_types.h"
#include "kritaimage_export.h"


class KRITAIMAGE_EXPORT KisCropSavedExtraData : public KUndo2CommandExtraData
{
public:
    enum Type {
        CROP_IMAGE,
        RESIZE_IMAGE,
        CROP_LAYER
    };

public:

    KisCropSavedExtraData(Type type, QRect cropRect, KisNodeSP cropNode = 0);
    ~KisCropSavedExtraData() override;

    inline Type type() const {
        return m_type;
    }

    inline QRect cropRect() const {
        return m_cropRect;
    }

    inline KisNodeSP cropNode() const {
        return m_cropNode;
    }

    KUndo2CommandExtraData* clone() const override {
        return new KisCropSavedExtraData(*this);
    }

private:
    Type m_type;
    QRect m_cropRect;
    KisNodeSP m_cropNode;
};

#endif /* __KIS_CROP_SAVED_EXTRA_DATA_H */
