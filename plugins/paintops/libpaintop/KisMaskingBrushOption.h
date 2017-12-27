/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISMASKINGBRUSHOPTION_H
#define KISMASKINGBRUSHOPTION_H

#include <kritapaintop_export.h>
#include <QScopedPointer>

#include <kis_types.h>
#include "kis_paintop_option.h"


class PAINTOP_EXPORT KisMaskingBrushOption : public KisPaintOpOption
{
public:
    typedef std::function<qreal()> MasterBrushSizeAdapter;

public:
    KisMaskingBrushOption(MasterBrushSizeAdapter masterBrushSizeAdapter);
    ~KisMaskingBrushOption() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void setImage(KisImageWSP image) override;

    void lodLimitations(KisPaintopLodLimitations *l) const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};



#endif // KISMASKINGBRUSHOPTION_H
