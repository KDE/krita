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

#include "kis_exif_value.h"

#include <kdebug.h>
#include <kcodecs.h>

namespace {
void set16Bit (unsigned char *data, ExifValue::ByteOrder order, const quint16* value)
{
    switch (order) {
        case ExifValue::BYTE_ORDER_MOTOROLA:
            data[0] = (unsigned char) (*value >> 8);
            data[1] = (unsigned char) *value;
            break;
        case ExifValue::BYTE_ORDER_INTEL:
            data[0] = (unsigned char) *value;
            data[1] = (unsigned char) (*value >> 8);
            break;
    }
}

void get16Bit (const unsigned char *data, ExifValue::ByteOrder order, quint16* value)
{
    switch (order) {
        case ExifValue::BYTE_ORDER_MOTOROLA:
            *value = ((data[0] << 8) | data[1]);
            break;
        case ExifValue::BYTE_ORDER_INTEL:
            *value = ((data[1] << 8) | data[0]);
            break;
    }
}

void get32Bit (const unsigned char *data, ExifValue::ByteOrder order, quint32* value)
{
    switch (order) {
        case ExifValue::BYTE_ORDER_MOTOROLA:
            *value = ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
        case ExifValue::BYTE_ORDER_INTEL:
            *value = ((data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0]);
    }
}

void set32Bit(unsigned char *data, ExifValue::ByteOrder order, const quint32* value)
{
    switch (order) {
        case ExifValue::BYTE_ORDER_MOTOROLA:
            data[0] = (unsigned char) (*value >> 24);
            data[1] = (unsigned char) (*value >> 16);
            data[2] = (unsigned char) (*value >> 8);
            data[3] = (unsigned char) *value;
            break;
        case ExifValue::BYTE_ORDER_INTEL:
            data[3] = (unsigned char) (*value >> 24);
            data[2] = (unsigned char) (*value >> 16);
            data[1] = (unsigned char) (*value >> 8);
            data[0] = (unsigned char) *value;
            break;
    }
}

void get64Bit (const unsigned char *data, ExifValue::ByteOrder order, quint64* value)
{
    switch (order) {
        case ExifValue::BYTE_ORDER_MOTOROLA:
            *value = (((quint64)data[0] << 56) | ((quint64)data[1] << 48) | ((quint64)data[2] << 40) | ((quint64)data[3] << 32) | ((quint64)data[4] << 24) | ((quint64)data[5] << 16) | ((quint64)data[6] << 8) | (quint64)data[7]);
        case ExifValue::BYTE_ORDER_INTEL:
            *value = (((quint64)data[7] << 56) | ((quint64)data[6] << 48) | ((quint64)data[5] << 40) | ((quint64)data[4] << 32) | ((quint64)data[3] << 24) | ((quint64)data[2] << 16) | ((quint64)data[1] << 8) | (quint64)data[0]);
    }
}

void set64Bit(unsigned char *data, ExifValue::ByteOrder order, const quint64* value)
{
    switch (order) {
        case ExifValue::BYTE_ORDER_MOTOROLA:
            data[0] = (unsigned char) (*value >> 56);
            data[1] = (unsigned char) (*value >> 48);
            data[2] = (unsigned char) (*value >> 40);
            data[3] = (unsigned char) (*value >> 32);
            data[4] = (unsigned char) (*value >> 24);
            data[5] = (unsigned char) (*value >> 16);
            data[6] = (unsigned char) (*value >> 8);
            data[7] = (unsigned char) *value;
            break;
        case ExifValue::BYTE_ORDER_INTEL:
            data[7] = (unsigned char) (*value >> 56);
            data[6] = (unsigned char) (*value >> 48);
            data[5] = (unsigned char) (*value >> 40);
            data[4] = (unsigned char) (*value >> 32);
            data[3] = (unsigned char) (*value >> 24);
            data[2] = (unsigned char) (*value >> 16);
            data[1] = (unsigned char) (*value >> 8);
            data[0] = (unsigned char) *value;
            break;
    }
}


}

ExifValue::ExifValue(ExifType ntype, unsigned char *data, unsigned int size, int ifd, uint ncomponents, ExifValue::ByteOrder order ) : m_ifd(ifd), m_type(ntype), m_components(ncomponents), m_value(0)
{
    allocData();
    setValue(data, size, order);
}

void ExifValue::allocData()
{
    if( type() != EXIF_TYPE_ASCII && type() != EXIF_TYPE_UNDEFINED)
    {
        m_value = new ExifNumber[components()];
    } else if ( type() == EXIF_TYPE_ASCII )
    {
        m_value = new QString();
    } else if ( type() == EXIF_TYPE_UNDEFINED)
    {
        m_value = new UByteArray();
    }
}

bool ExifValue::load(const QDomElement& elmt)
{
    QString attr;
    if( (attr = elmt.attribute("ifd")).isNull() )
        return false;
    m_ifd = attr.toInt();
    if( (attr = elmt.attribute("components")).isNull() )
        return false;
    m_components = attr.toInt();
    if( (attr = elmt.attribute("type")).isNull() )
        return false;
    m_type = (ExifValue::ExifType)attr.toInt();
    allocData();
    switch(type())
    {
        case EXIF_TYPE_BYTE:
            for(uint i = 0; i < components(); i++)
            {
                if( (attr = elmt.attribute(QString("value%1").arg(i) ) ).isNull() )
                {
                    setValue(i, (quint8)0);
                } else {
                    setValue(i, (quint8) attr.toUInt());
                }
            }
            break;
        case EXIF_TYPE_ASCII:
            setAsAscii( elmt.attribute("value" ) );
            break;
        case EXIF_TYPE_SHORT:
            for(uint i = 0; i < components(); i++)
            {
                if( (attr = elmt.attribute(QString("value%1").arg(i) ) ).isNull() )
                {
                    setValue(i, (quint16)0);
                } else {
                    setValue(i, (quint16) attr.toUInt());
                }
            }
            break;
        case EXIF_TYPE_LONG:
            for(uint i = 0; i < components(); i++)
            {
                if( (attr = elmt.attribute(QString("value%1").arg(i) ) ).isNull() )
                {
                    setValue(i, (quint32)0);
                } else {
                    setValue(i, (quint32) attr.toUInt());
                }
            }
            break;
        case EXIF_TYPE_RATIONAL:
            for(uint i = 0; i < components(); i++)
            {
                KisExifRational r;
                if( (attr = elmt.attribute(QString("numerator%1").arg(i) ) ).isNull() )
                {
                    r.numerator = (quint32)0;
                } else {
                    r.numerator = (quint32) attr.toUInt();
                }
                if( (attr = elmt.attribute(QString("denominator%1").arg(i) ) ).isNull() )
                {
                    r.denominator = (quint32)0;
                } else {
                    r.denominator = (quint32) attr.toUInt();
                }
                setValue(i, r);
            }
            break;
        case EXIF_TYPE_SBYTE:
            for(uint i = 0; i < components(); i++)
            {
                if( (attr = elmt.attribute(QString("value%1").arg(i) ) ).isNull() )
                {
                    setValue(i, (qint8)0);
                } else {
                    setValue(i, (qint8) attr.toInt());
                }
            }
            break;
        case EXIF_TYPE_UNDEFINED:
        {
            QString instr = elmt.attribute("value");
            QByteArray out;
            QByteArray in = instr.utf8();
            KCodecs::base64Decode( in, out);
            out.resize(out.size() - 2 );
            setAsUndefined((uchar*)out.data(), out.size() );
        }
        break;
        case EXIF_TYPE_SSHORT:
            for(uint i = 0; i < components(); i++)
            {
                if( (attr = elmt.attribute(QString("value%1").arg(i) ) ).isNull() )
                {
                    setValue(i, (qint16)0);
                } else {
                    setValue(i, (qint16) attr.toInt());
                }
            }
            break;
        case EXIF_TYPE_SLONG:
            for(uint i = 0; i < components(); i++)
            {
                if( (attr = elmt.attribute(QString("value%1").arg(i) ) ).isNull() )
                {
                    setValue(i, (qint32)0);
                } else {
                    setValue(i, (qint32) attr.toInt());
                }
            }
            break;
        case EXIF_TYPE_SRATIONAL:
            for(uint i = 0; i < components(); i++)
            {
                KisExifSRational r;
                if( (attr = elmt.attribute(QString("numerator%1").arg(i) ) ).isNull() )
                {
                    r.numerator = (qint32)0;
                } else {
                    r.numerator = (qint32) attr.toInt();
                }
                if( (attr = elmt.attribute(QString("denominator%1").arg(i) ) ).isNull() )
                {
                    r.denominator = (quint32)0;
                } else {
                    r.denominator = (quint32) attr.toInt();
                }
                setValue(i, r);
            }
            break;
        case EXIF_TYPE_FLOAT:
            for(uint i = 0; i < components(); i++)
            {
                if( (attr = elmt.attribute(QString("value%1").arg(i) ) ).isNull() )
                {
                    setValue(i, (float)0);
                } else {
                    setValue(i, (float) attr.toFloat());
                }
            }
            break;
        case EXIF_TYPE_DOUBLE:
            for(uint i = 0; i < components(); i++)
            {
                if( (attr = elmt.attribute(QString("value%1").arg(i) ) ).isNull() )
                {
                    setValue(i, (double)0);
                } else {
                    setValue(i, (double) attr.toDouble());
                }
            }
            break;
        case EXIF_TYPE_UNKNOW:
            break;

    }
    return true;
}

QDomElement ExifValue::save(QDomDocument& doc)
{
    QDomElement elmt = doc.createElement("ExifValue");
    elmt.setAttribute("ifd", ifd());
    elmt.setAttribute("components", components() );
    elmt.setAttribute("type", type() );
    switch(type())
    {
        case EXIF_TYPE_BYTE:
            for(uint i = 0; i < components(); i++)
                elmt.setAttribute(QString("value%1").arg(i), asByte( i ) );
            break;
        case EXIF_TYPE_ASCII:
            elmt.setAttribute("value", asAscii() );
            break;
        case EXIF_TYPE_SHORT:
            for(uint i = 0; i < components(); i++)
                elmt.setAttribute(QString("value%1").arg(i), asShort( i ) );
            break;
        case EXIF_TYPE_LONG:
            for(uint i = 0; i < components(); i++)
                elmt.setAttribute(QString("value%1").arg(i), asLong( i ) );
            break;
        case EXIF_TYPE_RATIONAL:
            for(uint i = 0; i < components(); i++)
            {
                KisExifRational r = asRational(i);
                elmt.setAttribute(QString("numerator%1").arg(i), r.numerator );
                elmt.setAttribute(QString("denominator%1").arg(i), r.denominator );
            }
            break;
        case EXIF_TYPE_SBYTE:
            for(uint i = 0; i < components(); i++)
                elmt.setAttribute(QString("value%1").arg(i), asSByte( i ) );
            break;
        case EXIF_TYPE_UNDEFINED:
        {
            UByteArray value = asUndefined();
            QByteArray data(value.size());
            data.setRawData((char*)value.data(), value.size());
            QByteArray encodedData;
            KCodecs::base64Encode( data, encodedData );
            elmt.setAttribute("value", QString(encodedData));
        }
            break;
        case EXIF_TYPE_SSHORT:
            for(uint i = 0; i < components(); i++)
                elmt.setAttribute(QString("value%1").arg(i), asSShort( i ) );
            break;
        case EXIF_TYPE_SLONG:
            for(uint i = 0; i < components(); i++)
                elmt.setAttribute(QString("value%1").arg(i), asSLong( i ) );
            break;
        case EXIF_TYPE_SRATIONAL:
            for(uint i = 0; i < components(); i++)
            {
                KisExifSRational r = asSRational(i);
                elmt.setAttribute(QString("numerator%1").arg(i), r.numerator );
                elmt.setAttribute(QString("denominator%1").arg(i), r.denominator );
            }
            break;
        case EXIF_TYPE_FLOAT:
            for(uint i = 0; i < components(); i++)
                elmt.setAttribute(QString("value%1").arg(i), asFloat( i ) );
            break;
        case EXIF_TYPE_DOUBLE:
            for(uint i = 0; i < components(); i++)
                elmt.setAttribute(QString("value%1").arg(i), asDouble( i ) );
            break;
        case EXIF_TYPE_UNKNOW:
            break;
    }
    return elmt;
}


void ExifValue::setValue(const unsigned char *data, unsigned int size, ExifValue::ByteOrder order)
{
    switch(type())
    {
        case EXIF_TYPE_BYTE:
            if( size == components() )
            {
                ExifNumber n;
                for(uint i = 0; i < components(); i++)
                {
                    n.m_byte = data[i];
                    setAsExifNumber( i, n);
                }
            }
            break;
        case EXIF_TYPE_ASCII:
            setAsAscii((char*) data);
            break;
        case EXIF_TYPE_SHORT:
            if( size == 2*components() )
            {
                ExifNumber n;
                for(uint i = 0; i < components(); i++)
                {
                    get16Bit( data + 2 * i, order, &n.m_short);
                    setAsExifNumber( i, n);
                }
            }
            break;
        case EXIF_TYPE_LONG:
            if( size == 4*components() )
            {
                ExifNumber n;
                for(uint i = 0; i < components(); i++)
                {
                    get32Bit( data + 4 * i, order, &n.m_long);
                    setAsExifNumber( i, n);
                }
            }
            break;
        case EXIF_TYPE_RATIONAL:
            if( size == 8*components() )
            {
                ExifNumber n;
                for(uint i = 0; i < components(); i++)
                {
                    get32Bit( data + 8 * i, order, &n.m_rational.numerator);
                    get32Bit( data + 8 * i + 4, order, &n.m_rational.denominator);
                    setAsExifNumber( i, n);
                }
            }
            break;
        case EXIF_TYPE_SBYTE:
            if( size == components() )
            {
                ExifNumber n;
                for(uint i = 0; i < components(); i++)
                {
                    n.m_sbyte = ((qint8*)data)[i];
                    setAsExifNumber( i, n);
                }
            }
            break;
        case EXIF_TYPE_UNDEFINED:
            setAsUndefined(data, size);
            break;
        case EXIF_TYPE_SSHORT:
            if( size == 2*components() )
            {
                ExifNumber n;
                for(uint i = 0; i < components(); i++)
                {
                    get16Bit( data + 2 * i, order, (quint16*)&n.m_sshort);
                    setAsExifNumber( i, n);
                }
            }
            break;
        case EXIF_TYPE_SLONG:
            if( size == 4*components() )
            {
                ExifNumber n;
                for(uint i = 0; i < components(); i++)
                {
                    get32Bit( data + 4 * i, order, (quint32*)&n.m_slong);
                    setAsExifNumber( i, n);
                }
            }
            break;
        case EXIF_TYPE_SRATIONAL:
            if( size == 8*components() )
            {
                ExifNumber n;
                for(uint i = 0; i < components(); i++)
                {
                    get32Bit( data + 8 * i, order, (quint32*)&n.m_srational.numerator);
                    get32Bit( data + 8 * i + 4, order, (quint32*)&n.m_srational.denominator);
                    setAsExifNumber( i, n);
                }
            }
            break;
        case EXIF_TYPE_FLOAT:
            if( size == 4*components() )
            {
                ExifNumber n;
                for(uint i = 0; i < components(); i++)
                {
                    get32Bit( data + 4 * i, order, (quint32*)&n.m_float);
                    setAsExifNumber( i, n);
                }
            }
            break;
        case EXIF_TYPE_DOUBLE:
            if( size == 8*components() )
            {
                ExifNumber n;
                for(uint i = 0; i < components(); i++)
                {
                    get64Bit( data + 8 * i, order, (quint64*)&n.m_double);
                    setAsExifNumber( i, n);
                }
            }
            break;
        case EXIF_TYPE_UNKNOW:
            break;
    }
}

void ExifValue::convertToData(unsigned char ** data, unsigned int* size, ExifValue::ByteOrder order)
{
    switch(type())
    {
        case EXIF_TYPE_BYTE:
            *size = components();
            *data = new uchar[*size];
            for(uint i = 0; i < components(); i++)
            {
                (*data)[i] = asExifNumber(i).m_byte;
            }
            return;
        case EXIF_TYPE_ASCII:
        {
            QString str = asAscii();
            *size = str.length();
            *data = new uchar[ *size ];
            uchar* ptr = *data;
            memcpy(ptr, str.ascii(), (*size)*sizeof(uchar));
        }
        return;
        break;
        case EXIF_TYPE_SHORT:
        {
            *size = 2*components();
            *data = new uchar[*size];
            for(uint i = 0; i < components(); i++)
            {
                set16Bit( (*data) + 2 * i, order, &asExifNumber(i).m_short);
            }
            return;
        }
        case EXIF_TYPE_LONG:
        {
            *size = 4*components();
            *data = new uchar[*size];
            for(uint i = 0; i < components(); i++)
            {
                set32Bit( (*data) + 4 * i, order, &asExifNumber(i).m_long);
            }
            return;
        }
        case EXIF_TYPE_RATIONAL:
            *size = 8*components();
            *data = new uchar[*size];
            for(uint i = 0; i < components(); i++)
            {
                ExifNumber n = asExifNumber(i);
                set32Bit( (*data) + 8 * i, order, &n.m_rational.numerator);
                set32Bit( (*data) + 8 * i + 4, order, &n.m_rational.denominator);
            }
            return;
        case EXIF_TYPE_SBYTE:
            *size = components();
            *data = new uchar[*size];
            for(uint i = 0; i < components(); i++)
            {
                *(((qint8*)*data) + i) = asExifNumber(i).m_sbyte;
            }
            return;
        case EXIF_TYPE_UNDEFINED:
        {
            UByteArray array = asUndefined();
            *size = array.size();
            *data = new uchar[*size];
            memcpy( *data, array.data(), (*size)*sizeof(unsigned char));
        }
        return;
        case EXIF_TYPE_SSHORT:
            *size = 2*components();
            *data = new uchar[*size];
            for(uint i = 0; i < components(); i++)
            {
                set16Bit( (*data) + 2 * i, order, (quint16*)&asExifNumber(i).m_sshort);
            }
            return;
        case EXIF_TYPE_SLONG:
            *size = 4*components();
            *data = new uchar[*size];
            for(uint i = 0; i < components(); i++)
            {
                set32Bit( (*data) + 4 * i, order, (quint32*)&asExifNumber(i).m_slong);
            }
            return;
        case EXIF_TYPE_SRATIONAL:
            *size = 8*components();
            *data = new uchar[*size];
            for(uint i = 0; i < components(); i++)
            {
                ExifNumber n = asExifNumber(i);
                set32Bit( (*data) + 4 * i, order, (quint32*)&asExifNumber(i).m_srational.numerator);
                set32Bit( (*data) + 4 * i + 4, order, (quint32*)&asExifNumber(i).m_srational.denominator);
            }
            return;
        case EXIF_TYPE_FLOAT:
            *size = 4*components();
            *data = new uchar[*size];
            for(uint i = 0; i < components(); i++)
            {
                set32Bit( (*data) + 4 * i, order, (quint32*)&asExifNumber(i).m_float);
            }
            return;
        case EXIF_TYPE_DOUBLE:
            *size = 8*components();
            *data = new uchar[*size];
            for(uint i = 0; i < components(); i++)
            {
                set64Bit( (*data) + 4 * i, order, (quint64*)&asExifNumber(i).m_double);
            }
            return;
        case EXIF_TYPE_UNKNOW:
            break;
    }
}

QString ExifValue::toString()
{
    switch(type())
    {
        case EXIF_TYPE_ASCII:
            return asAscii();
        case EXIF_TYPE_UNDEFINED:
        {
            QString undefined = "undefined";
            UByteArray array = asUndefined();
            for(uint i = 0; i < components(); i++)
            {
                undefined += "\\" + QString().setNum( array[i] );
            }
            return undefined;
        }
        default:
        {
            QString str = "";
            for(uint i = 0; i < components(); i++)
            {
                str += toString(i);
            }
            return str;
        }
    }
}

QString ExifValue::toString(uint i)
{
    switch(type())
    {
        case EXIF_TYPE_BYTE:
            return QString("%1 ").arg( asExifNumber( i ).m_byte );
        case EXIF_TYPE_SHORT:
            return QString("%1 ").arg( asExifNumber( i ).m_short );
        case EXIF_TYPE_LONG:
            return QString("%1 ").arg( asExifNumber( i ).m_long );
        case EXIF_TYPE_RATIONAL:
            return QString("%1 / %2 ").arg( asExifNumber( i ).m_rational.numerator ).arg( asExifNumber( i ).m_rational.denominator );
        case EXIF_TYPE_SBYTE:
            return QString("%1 ").arg( asExifNumber( i ).m_sbyte );
        case EXIF_TYPE_SSHORT:
            return QString("%1 ").arg( asExifNumber( i ).m_sshort );
        case EXIF_TYPE_SLONG:
            return QString("%1 ").arg( asExifNumber( i ).m_slong );
        case EXIF_TYPE_SRATIONAL:
            return QString("%1 / %2 ").arg( asExifNumber( i ).m_srational.numerator ).arg( asExifNumber( i ).m_srational.denominator );
        case EXIF_TYPE_FLOAT:
            return QString("%1 ").arg( asExifNumber( i ).m_float );
        case EXIF_TYPE_DOUBLE:
            return QString("%1 ").arg( asExifNumber( i ).m_double );
        default:
            return "unknow ";
    }
}

