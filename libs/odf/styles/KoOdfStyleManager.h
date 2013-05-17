/* This file is part of the KDE project
 *
 * Copyright (C) 2013 Inge Wallin <inge@lysator.liu.se>
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


#ifndef KOODF_STYLE_MANAGER_H
#define KOODF_STYLE_MANAGER_H

#include "koodf_export.h"


class QString;
class KoStore;
class KoOdfStyle;
class KoXmlStreamReader;
class KoXmlWriter;


class KOODF_EXPORT KoOdfStyleManager
{
 public:
    KoOdfStyleManager();
    ~KoOdfStyleManager();

    KoOdfStyle *style(QString &name) const;
    void setStyle(QString &name, KoOdfStyle *style);

    KoOdfStyle *defaultStyle(QString &family) const;
    void setDefaultStyle(QString &family, KoOdfStyle *style);

    void clear();

    bool loadStyles(KoStore *odfStore);
    bool saveNamedStyles(KoXmlWriter *writer);

 private:
    // FIXME: Move to private class.
    void collectStyleSet(KoXmlStreamReader &reader);

 private:
    class Private;
    Private * const d;
};


#endif
