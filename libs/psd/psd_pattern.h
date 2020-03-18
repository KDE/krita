/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef PSD_PATTERN_H
#define PSD_PATTERN_H

#include "psd.h"
#include <QIODevice>
#include <resources/KoPattern.h>

class KRITAPSD_EXPORT PsdPattern
{
public:
    PsdPattern();
    ~PsdPattern();

    void setPattern(KoPatternSP pattern);
    KoPatternSP pattern() const;

    bool psd_write_pattern(QIODevice* io);
    bool psd_read_pattern(QIODevice* io);
private:
    struct Private;
    Private * const d;
};

#endif // PSD_PATTERN_H
