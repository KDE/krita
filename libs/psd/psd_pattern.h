/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
