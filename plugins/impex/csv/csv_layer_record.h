/*
 *  Copyright (c) 2016 Laszlo Fazekas <mneko@freemail.hu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
