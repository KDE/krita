/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#ifndef DDEDATA_H
#define DDEDATA_H

/**
 * Contains the DDE information for this table. DDE only works on Windows,
 * and  KOffice doesn't support it, but we need to keep it around for rountrip
 * purposes.
 */
struct DDEData{

    enum ConversionMode {
        IntoDefaultDataStyle,
        IntoEnglishNumber,
        KeepText
    };

    QString ddeApplication;
    QString ddeTopic;
    QString ddeItem;
    bool automaticUpdate;
    QString connectionName;
    ConversionMode conversionMode;

};

#endif
