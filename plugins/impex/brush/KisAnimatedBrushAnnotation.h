/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISANIMATEDBRUSHANNOTATION_H
#define KISANIMATEDBRUSHANNOTATION_H

#include <kis_annotation.h>

class KisPipeBrushParasite;

class KisAnimatedBrushAnnotation : public KisAnnotation
{
public:
    KisAnimatedBrushAnnotation(const KisPipeBrushParasite &parasite);
    KisAnnotation* clone() const Q_DECL_OVERRIDE {
        return new KisAnimatedBrushAnnotation(*this);
    }
};

#endif // KISANIMATEDBRUSHANNOTATION_H
