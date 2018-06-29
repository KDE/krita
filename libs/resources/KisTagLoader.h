/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISTAGLOADER_H
#define KISTAGLOADER_H

/**
 * @brief The KisTagLoader loads a tag from a .tag file.
 * A .tag file is a .desktop file. The following fields
 * are important:
 *
 * name: the name of the tag, which can be translated
 * comment: a tooltip for the tag, which can be translagted
 * url: the untranslated name of the tag
 */
class KisTagLoader
{
public:
    KisTagLoader();
};

#endif // KISTAGLOADER_H
