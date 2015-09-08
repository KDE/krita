/*
 *  Copyright (c) 2010 Valek Filippov <frob@gnome.org>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <QString>
#include <QFile>
#include <QDataStream>
#include <kis_debug.h>
#include <iostream>

const QString PATT = "patt";
const QString DESC = "desc";
const QString VLLS = "VlLs";
const QString TEXT = "TEXT";
const QString OBJC = "Objc";
const QString UNTF = "UntF";
const QString BOOL = "bool";
const QString LONG = "long";
const QString DOUB = "doub";
const QString ENUM = "enum";

enum enumFuncNames  {
    P_PATT,
    P_DESC,
    P_VLLS,
    P_TEXT,
    P_OBJC,
    P_UNTF,
    P_BOOL,
    P_LONG,
    P_DOUB,
    P_ENUM
};


static QHash<QString, enumFuncNames> types;

static QString p_patt(QDataStream &buf)
{
    // didn't rev.engineered yet
    Q_UNUSED(buf);
    return QString("");
}

static QString p_desc(QDataStream &buf)
{
    // convert 4 bytes as big-endian unsigned long
    quint32 size;
    // 22 + 4
    buf >> size;
    buf.skipRawData(22);
    return QString::number(size);
}

static QString p_vlls(QDataStream &buf)
{
    quint32 size;
    buf >> size;
    return QString::number(size);
}

static QString p_text(QDataStream &buf)
{
    quint32 size;
    buf >> size;

    ushort * text = new ushort[size];
    for (int i = 0; i < size; i++) {
        buf >> text[i];
    }
    return QString::fromUtf16(text, size);
}

static QString p_objc(QDataStream &buf)
{
    quint32 objvallen;
    buf >> objvallen;

    char * objval = new char[objvallen * 2 + 1];
    buf.readRawData(objval, objvallen * 2);
    objval[ objvallen * 2 ] = '\0';

    quint32 size;
    buf >> size;
    if (size == 0) {
        size = 4;
    }

    char * name = new char[size + 1];

    buf.readRawData(name, size);
    name[size] = '\0';

    quint32 value;
    buf >> value;
    return QString::fromLatin1(name) + ' ' + QString::number(value);
}

static QString p_untf(QDataStream &buf)
{
    char * type = new char[5];
    buf.readRawData(type, 4);
    type[4] = '\0';
    double value;
    buf >> value;
    return QString::fromLatin1(type) + ' ' + QString::number(value);
}

static QString p_bool(QDataStream &buf)
{
    //# ord converts 1 byte number
    char byte;
    buf.device()->getChar(&byte);
    if (byte) return QString("1");
    else return QString("0");
}

static QString p_doub(QDataStream &buf)
{
    // unpack 8 bytes ieee 754 value to floating point number
    double value;
    buf >> value;
    return QString::number(value);
}

static QString p_enum(QDataStream &buf)
{
    quint32 size1, size2;
    buf >> size1;

    if (size1 == 0) {
        size1 = 4;
    }
    char * name1 = new char[size1 + 1];
    buf.readRawData(name1, size1);
    name1[size1] = '\0';

    buf >> size2 ;
    if (size2 == 0) {
        size2 = 4;
    }

    char * name2 = new char[size2 + 1];
    buf.readRawData(name2, size2);
    name2[size2] = '\0';

    return QString::fromLatin1(name1) + ' ' + QString::fromLatin1(name2);
}

static quint32 parseEntry(QDataStream &buf)
{
    quint32 nlen;

    QString value;

    buf >> nlen;
    if (nlen == 0) {
        nlen = 4;
    }

    if (nlen == 1331849827) { // "Objc"
        value = p_objc(buf); // TODO: port
        dbgKrita << "Objc " << value;
    }
    else {
        // read char with nlen bytes and convert to String
        char * name = new char[ nlen + 1 ];
        int status = buf.readRawData(name, nlen);
        if (status == -1) {
            dbgKrita << "Error, name can't be readed";
        }
        name[nlen] = '\0';

        char * type = new char[5];
        status = buf.readRawData(type, 4);
        type[4] = '\0';
        QString key = QString::fromLatin1(type);

        if (types.contains(key)) {
            enumFuncNames enumName = types[key];

            switch (enumName) {
            case P_PATT: value = p_patt(buf); break;
            case P_DESC: value = p_desc(buf); break;
            case P_VLLS: value = p_vlls(buf); break;
            case P_TEXT: value = p_text(buf); break;
            case P_OBJC: value = p_objc(buf); break;
            case P_UNTF: value = p_untf(buf); break;
            case P_BOOL: value = p_bool(buf); break;
            case P_LONG: value = p_vlls(buf); break; // yes vlls, it is not typo
            case P_DOUB: value = p_doub(buf); break;
            case P_ENUM: value = p_enum(buf); break;
            default: dbgKrita << "Freak error occurred!"; break;
            }
            dbgKrita << name << type << value;

        }
        else {
            dbgKrita << "Unknown key:\t" << name << type;
            //dbgKrita << p_unkn(buf);
            return -1;
        }

    }
    return 0;
}


static void parse(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        dbgKrita << "Can't open file " << fileName;
        return;
    }
    QDataStream buf(&file);


    // offset in bytes
    short int vermaj, vermin;
    buf >> vermaj;
    buf >> vermin;
    dbgKrita << "Version: " << vermaj << "." << vermin;

    int index = file.readAll().indexOf("8BIMdesc");
    buf.device()->seek(index);
    int status = 0;
    while (!buf.atEnd()) {
        status = parseEntry(buf);
        if (status == -1) {
            // something to break the parsing with fail?
            dbgKrita << "Finishing with fail...";
            break;
        }
    }
}

int main(int argc, const char * argv[])
{
    QString fileName;
    if (argc != 2) {
        fileName = "test.abr";
    }
    else {
        fileName = QString::fromLatin1(argv[1]);
    }


    types.insert(PATT, P_PATT);
    types.insert(DESC, P_DESC);
    types.insert(VLLS, P_VLLS);
    types.insert(TEXT, P_TEXT);
    types.insert(OBJC, P_OBJC);
    types.insert(UNTF, P_UNTF);
    types.insert(BOOL, P_BOOL);
    types.insert(LONG, P_LONG);
    types.insert(DOUB, P_DOUB);
    types.insert(ENUM, P_ENUM);

    parse(fileName);

    return 0;
}
