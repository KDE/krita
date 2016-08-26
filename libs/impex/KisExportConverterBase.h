/* This file is part of the KDE project
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KISEXPORTCONVERTERBASE_H
#define KISEXPORTCONVERTERBASE_H

#include <kis_types.h>

/**
 * @brief The KisExportConverterBase class defines the interface
 * of converter classes that can convert an aspect of an image
 * before exporting.
 */
class KisExportConverterBase
{
public:
    KisExportConverterBase();
    virtual ~KisExportConverterBase();
    virtual QString id() const = 0;
    virtual bool convert(KisImageSP image) const = 0;
    virtual int priority() const = 0;

};

#endif // KISEXPORTCONVERTERBASE_H
