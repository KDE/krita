/* This file is part of the KDE project
 * Copyright 2008 (C) Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_KRA_SAVER
#define KIS_KRA_SAVER

#include <kis_types.h>

class KisDoc2;
class QDomElement;
class QDomDocument;
class KoStore;
class QString;

class KisKraSaver
{
public:

    KisKraSaver(KisDoc2* document);

    ~KisKraSaver();

    QDomElement saveXML(QDomDocument& doc,  KisImageWSP image);

    bool saveBinaryData(KoStore* store, KisImageWSP image, const QString & uri, bool external);

private:

    class Private;
    Private * const m_d;
};

#endif
