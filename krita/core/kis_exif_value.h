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
#ifndef KIS_EXIF_VALUE_H
#define KIS_EXIF_VALUE_H

#include <qdom.h> 

#include <q3cstring.h>
#include <qstring.h>
//Added by qt3to4:
#include <Q3MemArray>

typedef Q3MemArray<quint8> UByteArray;

struct KisExifRational {
    quint32 numerator;
    quint32 denominator;
};

struct KisExifSRational {
    qint32 numerator;
    qint32 denominator;
};

class ExifValue {
    typedef union {
        quint8 m_byte;
        quint16 m_short;
        quint32 m_long;
        KisExifRational m_rational;
        qint8 m_sbyte;
        qint16 m_sshort;
        qint32 m_slong;
        KisExifSRational m_srational;
        float m_float;
        double m_double;
    } ExifNumber;
    public:
        enum ExifType {
            EXIF_TYPE_BYTE       =  1,
            EXIF_TYPE_ASCII      =  2,
            EXIF_TYPE_SHORT      =  3,
            EXIF_TYPE_LONG       =  4,
            EXIF_TYPE_RATIONAL   =  5,
            EXIF_TYPE_SBYTE      =  6,
            EXIF_TYPE_UNDEFINED  =  7,
            EXIF_TYPE_SSHORT     =  8,
            EXIF_TYPE_SLONG      =  9,
            EXIF_TYPE_SRATIONAL  = 10,
            EXIF_TYPE_FLOAT      = 11,
            EXIF_TYPE_DOUBLE     = 12,
            EXIF_TYPE_UNKNOW     = 13
        };
        enum ByteOrder {
            BYTE_ORDER_MOTOROLA,
            BYTE_ORDER_INTEL
        };
        ExifValue() : m_ifd(-1), m_type(EXIF_TYPE_UNKNOW), m_components(0), m_value(0) { }
        ExifValue(ExifType type, unsigned char *data, unsigned int size, int ifd, uint components, ExifValue::ByteOrder order);
       	virtual ~ExifValue() {} 
        virtual bool load(const QDomElement& elmt);
        virtual QDomElement save(QDomDocument& doc);

        /**
         * Return the type of the array
         */
        inline ExifType type() { return m_type; }
        inline const UByteArray asUndefined() {
            if(m_type == EXIF_TYPE_UNDEFINED)
                return *(UByteArray*) m_value;
            return UByteArray();
        }
        inline void setAsUndefined(const unsigned char *data, unsigned int size)
        {
            if(m_type == EXIF_TYPE_UNDEFINED)
            {
                ((UByteArray*)m_value)->duplicate(data, size);
                m_components = size;
            }
        }
        inline const QString asAscii() {
            if(m_type == EXIF_TYPE_ASCII)
                return QString(*(QString*) m_value);
            return QString();
        }
        inline void setAsAscii(char* data)
        {
            if(m_type == EXIF_TYPE_ASCII)
            {
                QString str = QString((char*) data);
                *(QString*)m_value = str;
                m_components = str.length();
            }
        }
        inline void setAsAscii(QString str)
        {
            *(QString*)m_value = str;
            m_components = str.length();
        }
        void convertToData(unsigned char ** data, unsigned int* size, ExifValue::ByteOrder order);
        /**
         * Return the ifd number to which this ExifValue belongs.
         */
        inline int ifd() { return m_ifd; }
        /**
         * Return the number of components of this ExifValue
         */
        inline uint components() { return m_components; }
        
        /**
         * This function return the value of a the ExifValue as a string.
         */
        QString toString();
        
        inline quint8 asByte(uint i)
        {
            if(m_type == EXIF_TYPE_BYTE)
                return asExifNumber(i).m_byte;
            return 0;
        }
        inline void setValue(uint i, quint8 v)
        {
            ((ExifNumber*)m_value)[i].m_byte = v;
        }
        inline quint8 asShort(uint i)
        {
            if(m_type == EXIF_TYPE_SHORT)
                return asExifNumber(i).m_short;
            return 0;
        }
        inline void setValue(uint i, quint16 v)
        {
            ((ExifNumber*)m_value)[i].m_short = v;
        }
        inline quint8 asLong(uint i)
        {
            if(m_type == EXIF_TYPE_LONG)
                return asExifNumber(i).m_long;
            return 0;
        }
        inline void setValue(uint i, quint32 v)
        {
            ((ExifNumber*)m_value)[i].m_long = v;
        }
        inline KisExifRational asRational(uint i)
        {
            if(m_type == EXIF_TYPE_RATIONAL)
                return asExifNumber(i).m_rational;
            return KisExifRational();
        }
        inline void setValue(uint i, quint32 n, quint32 d)
        {
            ((ExifNumber*)m_value)[i].m_rational.numerator = n;
            ((ExifNumber*)m_value)[i].m_rational.denominator = d;
        }
        inline void setValue(uint i, KisExifRational r)
        {
            ((ExifNumber*)m_value)[i].m_rational = r;
        }
        inline qint8 asSByte(uint i)
        {
            if(m_type == EXIF_TYPE_SBYTE)
                return asExifNumber(i).m_sbyte;
            return 0;
        }
        inline void setValue(uint i, qint8 v)
        {
            ((ExifNumber*)m_value)[i].m_sbyte = v;
        }
        inline qint16 asSShort(uint i)
        {
            if(m_type == EXIF_TYPE_SSHORT)
                return asExifNumber(i).m_sshort;
            return 0;
        }
        inline void setValue(uint i, qint16 v)
        {
            ((ExifNumber*)m_value)[i].m_sshort = v;
        }
        inline qint32 asSLong(uint i)
        {
            if(m_type == EXIF_TYPE_SLONG)
                return asExifNumber(i).m_slong;
            return 0;
        }
        inline void setValue(uint i, qint32 v)
        {
            ((ExifNumber*)m_value)[i].m_slong = v;
        }
        inline KisExifSRational asSRational(uint i)
        {
            if(m_type == EXIF_TYPE_SRATIONAL)
                return asExifNumber(i).m_srational;
            return KisExifSRational();
        }
        inline void setValue(uint i, KisExifSRational r)
        {
            ((ExifNumber*)m_value)[i].m_srational = r;
        }
        inline void setValue(uint i, qint32 n, qint32 d)
        {
            ((ExifNumber*)m_value)[i].m_srational.numerator = n;
            ((ExifNumber*)m_value)[i].m_srational.denominator = d;
        }
        inline float asFloat(uint i)
        {
            if(m_type == EXIF_TYPE_FLOAT)
                return asExifNumber(i).m_float;
            return 0.;
        }
        inline void setValue(uint i, float v)
        {
            ((ExifNumber*)m_value)[i].m_float = v;
        }
        inline double asDouble(uint i)
        {
            if(m_type == EXIF_TYPE_DOUBLE)
                return asExifNumber(i).m_double;
            return 0.;
        }
        inline void setValue(uint i, double v)
        {
            ((ExifNumber*)m_value)[i].m_double = v;
        }
    private:
        /**
         * Return the ith component as a string.
         */
        QString toString(uint i);
        void setValue(const unsigned char *data, unsigned int size, ExifValue::ByteOrder order);
        /**
         * Return the ExifValue as a number.
         */
        inline const ExifNumber asExifNumber(uint index)
        {
            Q_ASSERT(index < m_components);
            return ((ExifNumber*)m_value)[index];
        }
        inline void setAsExifNumber(uint index, ExifNumber n)
        {
            Q_ASSERT(index < m_components);
            ((ExifNumber*)m_value)[index] = n;
        }
        /**
         * This function will allocate the memory used for storing the current data.
         */
        void allocData();
    private:
        int m_ifd;
        ExifType m_type;
        uint m_components;
        void *m_value;
};

#endif
