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

#include "kis_color_picker_stroke_strategy.h"

#include <KoColor.h>


#include "kis_debug.h"
#include "kis_tool_utils.h"
#include <kis_wrapped_rect.h>
#include "kis_default_bounds.h"
#include "kis_paint_device.h"


struct KisColorPickerStrokeStrategy::Private
{
    Private() : shouldSkipWork(false) {}

    bool shouldSkipWork;
};

KisColorPickerStrokeStrategy::KisColorPickerStrokeStrategy()
    : m_d(new Private)
{
    setSupportsWrapAroundMode(true);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
}

KisColorPickerStrokeStrategy::~KisColorPickerStrokeStrategy()
{
}

void KisColorPickerStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    if (m_d->shouldSkipWork) return;

    Data *d = dynamic_cast<Data*>(data);
    KIS_ASSERT_RECOVER_RETURN(d);

    KoColor color;
    bool result = KisToolUtils::pick(d->dev, d->pt, &color);
    Q_UNUSED(result);

    emit sigColorUpdated(color);
}

KisStrokeStrategy* KisColorPickerStrokeStrategy::createLodClone(int levelOfDetail)
{
    Q_UNUSED(levelOfDetail);

    m_d->shouldSkipWork = true;

    KisColorPickerStrokeStrategy *lodStrategy = new KisColorPickerStrokeStrategy();
    connect(lodStrategy, &KisColorPickerStrokeStrategy::sigColorUpdated,
            this, &KisColorPickerStrokeStrategy::sigColorUpdated,
            Qt::DirectConnection);
    return lodStrategy;
}

