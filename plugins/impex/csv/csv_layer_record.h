/*
 *  SPDX-FileCopyrightText: 2016 Laszlo Fazekas <mneko@freemail.hu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CSV_LAYER_RECORD_H_
#define CSV_LAYER_RECORD_H_

#include <QString>

#include "kis_raster_keyframe_channel.h"

class CSVLayerRecord
{
public:
    CSVLayerRecord();
    virtual ~CSVLayerRecord();

    QString     name;
    QString     blending;
    float       density {0.0};
    int         visible {0};

    KisLayer*   layer {0};
    KisRasterKeyframeChannel *channel {0};
    QString     last;
    QString     path;
    int         frame {0};
};

#endif
