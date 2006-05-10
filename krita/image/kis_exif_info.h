/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_DOCUMENT_INFO_EXIF_H
#define KIS_DOCUMENT_INFO_EXIF_H

#include "kis_exif_value.h"

#include <qdom.h> 
#include <QMap>

class KisExifInfo
{
    public:
        KisExifInfo();
        virtual ~KisExifInfo();

        virtual bool load(const QDomElement& elmt);
        virtual QDomElement save(QDomDocument& doc);

        bool getValue(QString name, ExifValue& value)
        {
            if ( m_values.find( name ) == m_values.end() ) {
                return false;
            }
            else {
                value = m_values[name];
                return true;
            }
        }
        void setValue(QString name, ExifValue value)
        {
            m_values[name] = value;
        }
        typedef QMap<QString, ExifValue> evMap;
        evMap::const_iterator begin() const { return m_values.begin(); }
        evMap::const_iterator end() const { return m_values.end(); }
    private:
        evMap m_values;
};

#endif
