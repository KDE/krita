/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
