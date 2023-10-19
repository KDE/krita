/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_cos_parser.h"

#include <QTextCodec>
#include <QVariantHash>
#include <QVariantList>
#include <QBuffer>
#include <QVariant>
#include <QDebug>

enum {
    Null = 0x00,
    Space = 0x20,
    Tab = 0x09,
    LineFeed = 0x0a,
    FormFeed = 0x0c,
    Return = 0x0d,
    BeginArray = 0x5b, // [
    BeginObject = 0x3c, // <
    EndArray = 0x5d, // ]
    EndObject = 0x3e, // >
    BeginName = 0x2f, // /
    BeginString = 0x28, // (
    EndString = 0x29, // )
    ByteOrderMark = 0xfe
};

bool isWhiteSpace(char c) {
    switch (c) {
    case Null:
    case Tab:
    case LineFeed:
    case FormFeed:
    case Return:
    case Space:
        return true;
    default:
        return false;
    }
}

void eatSpace(QIODevice &dev) {
    //qDebug() << Q_FUNC_INFO;
    char c;
    dev.peek(&c, 1);
    while (isWhiteSpace(c) && !dev.atEnd()) {
        dev.skip(1);
        dev.peek(&c, 1);
    }
}

bool parseName(QIODevice &dev, QVariant &val) {
    //qDebug() << Q_FUNC_INFO;
    char c;
    dev.getChar(&c);
    QString name = "/"; // prepending with / so that we know this is a name.

    while (c >= 0x21 && c<0x7e && !isWhiteSpace(c) && c != BeginName && !dev.atEnd()) {
        name.append(c);
        dev.getChar(&c);
    }

    if (c == BeginName) {
        dev.ungetChar(c);
    }
    val = QVariant(name);
    return true;
}

const QMap<char, char> escaped = {
    {'n', LineFeed},
    {'r', Return},
    {'t', Tab},
    {'b', 0x08}, // backspace
    {'f', FormFeed},
    {'(', BeginString},
    {')', EndString},
    {'\\', 0x5c} // reverse solidus/backslash
};

bool parseString(QIODevice &dev, QVariant &val) {
    //qDebug() << Q_FUNC_INFO;
    char c;
    dev.getChar(&c);
    QByteArray text;

    while(c != EndString && !dev.atEnd()) {
        if (c == '\\') {

            // Try to parse PDF \ddd notation.
            char c2;
            dev.peek(&c2, 1);

            if (escaped.keys().contains(c2)) {
                text.append(escaped.value(c2));
                dev.skip(1);
            } else {
                QByteArray octal;
                for (int i=0; i<3; i++) {
                    char c2;
                    dev.peek(&c2, 1);
                    if (c2 >= '0' && c2 <= '9') {
                        octal.append(c2);
                        dev.skip(1);
                    }
                }
                bool ok;
                int val = octal.toInt(&ok, 8);
                if (ok) {
                    qDebug() << "escaped octal" << val;
                    // don't know how to actually interpret this as a char...
                } else {
                    text.append(c);
                    text.append(octal);

                }
            }
        } else {
            text.append(c);
        }
        dev.getChar(&c);
    }

    //qDebug() << text;

    if (text.startsWith(ByteOrderMark)) {
        QTextCodec *Utf16Codec = QTextCodec::codecForName("UTF-16BE");
        val = Utf16Codec->toUnicode(text);
    } else {
        val = QString::fromLatin1(text);
    }

    return true;
}

bool parseHexString(QIODevice &dev, QVariant &val) {
    //qDebug() << Q_FUNC_INFO;
    char c;
    dev.getChar(&c);
    QByteArray hex;

    while (c!= EndObject && !dev.atEnd()) {
        hex.append(c);
        dev.getChar(&c);
    }

    val = QString("<"+QString::fromLatin1(hex)+">");
    return true;
}

bool parseNumber(QIODevice &dev, QVariant &val) {
    //qDebug() << Q_FUNC_INFO;
    char c;
    bool isDouble = false;
    dev.getChar(&c);
    QString number;
    if (c == '-' || c == '+') {
        number.append(c);
        dev.getChar(&c);
    }
    while (c >= '0' && c <= '9') {
        number.append(c);
        dev.getChar(&c);
    }
    if (c == '.') {
        isDouble = true;
        number.append(c);
        dev.getChar(&c);
        while (c >= '0' && c <= '9') {
            number.append(c);
            dev.getChar(&c);
        }
    }
    //qDebug() << number << c;

    bool ok;
    if (isDouble) {
        val = number.toDouble(&ok);
    } else {
        val = number.toInt(&ok);
    }
    return ok;
}

bool KisCosParser::parseObject(QIODevice &dev, QVariantHash &object, bool checkEnd) {
    //qDebug() << Q_FUNC_INFO;
    eatSpace(dev);

    QVariant key;
    QVariant val;
    while (parseValue(dev, key)) {
        //qDebug() << key;
        object.insert(key.toString(), QVariant());
        if (key.type() == QVariant::String && parseValue(dev, val)) {
            object.insert(key.toString(), val);
        } else {
            return false;
        }
    }
    char c;
    dev.getChar(&c);
    if (c == EndObject) {
        dev.skip(1);
        return true;
    } else if (checkEnd) {
        return false;
    }

    return true;
}

bool KisCosParser::parseArray(QIODevice &dev, QVariantList &array)
{
    //qDebug() << Q_FUNC_INFO;
    eatSpace(dev);

    QVariant val;
    while (parseValue(dev, val)) {
        array.append(val);
    }
    char c;
    dev.getChar(&c);
    if (c == EndArray) {
        return true;
    } else {
        return false;
    }

    return true;
}

bool KisCosParser::parseValue(QIODevice &dev, QVariant &val) {
    //qDebug() << Q_FUNC_INFO;

    eatSpace(dev);
    char c;
    dev.getChar(&c);

    if (c == BeginObject) {
        char c2;
        dev.peek(&c2, 1);
        if (c2 == BeginObject) {
            QVariantHash object = QVariantHash();
            dev.skip(1);
            if (!parseObject(dev, object)) {
                return false;
            }
            val = object;
        } else {
            if (!parseHexString(dev, val)) {
                return false;
            }
        }
    } else if (c == BeginArray) {
        QVariantList array = QVariantList();
        if (!parseArray(dev, array)) {
            return false;
        }
        val = array;
    } else if (c == BeginName) {
        if (!parseName(dev, val)) {
            return false;
        }
    } else if (c == BeginString) {
        if (!parseString(dev, val)) {
            return false;
        }
    } else if (c == EndObject || c == EndArray) {
        dev.ungetChar(c);
        return false;
    } else if (c == 't') {
        QByteArray t;
        dev.read(t.data(), 3);
        if (t[0] == 'r' && t[1] == 'u' && t[2] == 'e') {
            val = true;
        } else {
            return false;
        }

    } else if (c == 'f') {
        QByteArray t;
        dev.read(t.data(), 4);
        if (t[0] == 'a' && t[1] == 'l' && t[2] == 's' && t[3] == 'e') {
            val = false;
        } else {
            return false;
        }
    } else if (c == 'n') {
        QByteArray t;
        dev.read(t.data(), 3);
        if (t[0] == 'u' && t[1] == 'l' && t[2] == 'l') {
            val = QVariant();
        } else {
            return false;
        }
    } else {
        dev.ungetChar(c);
        if (!parseNumber(dev, val)) {
            return false;
        }
    }

    return true;
}

QVariantHash KisCosParser::parseCosToJson(QByteArray *ba)
{
    QVariant root;
    QBuffer dev(ba);
    if (dev.open(QIODevice::ReadOnly)) {

        eatSpace(dev);
        char c;
        dev.peek(&c, 1);
        if (c == BeginObject) {
            //qDebug() << "parsing as value";
            if (!parseValue(dev, root)) {
                qDebug() << "dev not at end";
            }
        } else {
            QVariantHash b;
            if (!parseObject(dev, b, false)) {
                qDebug() << "txt2 dev not at end";
            }
            root = b;
        }
        dev.close();
    }
    return root.toHash();
}
