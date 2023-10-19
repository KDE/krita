/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_cos_writer.h"

#include <QTextCodec>
#include <QVariantHash>
#include <QVariant>
#include <QVariantList>
#include <QBuffer>

const QMap<char, char> escape = {
    {0x0a, 'n'},
    {0x0d, 'r'},
    {0x09, 't'},
    {0x08, 'b'}, // backspace
    {0x0c, 'f'},
    {0x28, '('},
    {0x29, ')'},
    {0x5c, '\\'} // reverse solidus/backslash
};

// Because we can't differentiate between names and regular strings,
// we'll just list the keys of the names whose value is also a name.
static QStringList nameKeys {
    "/StreamTag",
    "/ListStyle",
    "/MojiKumiTable",
    "/KurikaeshiMojiShori"
};

void writeString(QIODevice &dev, const QVariant val, const QString name) {
    QString newString = val.toString();
    if (nameKeys.contains(name)) {
        dev.write((name+" "+newString).toLatin1());
    } else {
        newString.replace("\n", "\r");
        QTextCodec *Utf16Codec = QTextCodec::codecForName("UTF-16BE");
        dev.write((name+" (").toLatin1());
        QByteArray unicode = Utf16Codec->fromUnicode(newString);
        QByteArray escaped = QByteArray();

        char *c = unicode.begin();
        while(c < unicode.end()) {
            if (escape.keys().contains(*c)) {
                escaped.append('\\');
                escaped.append(escape.value(*c));
            } else {
                escaped.append(*c);
            }
            c++;
        }
        qDebug() << name << newString;
        qDebug() << escaped;
        dev.write(escaped);
        dev.write(")");
    }
}

void writeVariant(QIODevice &dev, const QVariantHash object, int indent, bool prettyPrint) {
    QByteArray indentStringOld(indent, QChar::Tabulation);
    QString newLine = prettyPrint? "\n": " ";
    dev.write(indentStringOld);
    dev.write(("<<"+newLine).toLatin1());
    indent = prettyPrint? indent + 1: 0;
    QByteArray indentString(indent, QChar::Tabulation);
    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object[key];
        QString name = QString(key);
        if (val.type() == QVariant::Hash) {
            dev.write(indentString);
            dev.write((name+newLine).toLatin1());
            writeVariant(dev, val.toHash(), indent, prettyPrint);
        } else if (val.type() == QVariant::List) {
            QVariantList array = val.toList();
            if(array.isEmpty()) {
                dev.write(indentString);
                dev.write((name+" [ ]"+newLine).toLatin1());
            } else if (array.at(0).type() == QVariant::Int) {
                dev.write(indentString);
                dev.write((name+" [").toLatin1());
                for (int i=0; i<array.size(); i++) {
                    dev.write((" "+QString::number(array.at(i).toInt())).toLatin1());
                }
                dev.write((" ]"+newLine).toLatin1());
            } else if (array.at(0).type() == QVariant::Double) {
                dev.write(indentString);
                dev.write((name+" [").toLatin1());
                for (int i=0; i<array.size(); i++) {
                    dev.write((" "+QString::number(array.at(i).toDouble(), 'f', 5)).toLatin1());
                }
                dev.write((" ]"+newLine).toLatin1());
            } else {
                dev.write(indentString);
                dev.write((name+" ["+newLine).toLatin1());
                for (int i=0; i<val.toList().size(); i++) {
                    QVariant arrVal = val.toList().at(i);
                    if (arrVal.type() == QVariant::Hash) {
                        writeVariant(dev, arrVal.toHash(), indent, prettyPrint);
                    }
                }
                dev.write(indentString);
                dev.write(("]"+newLine).toLatin1());
            }
        } else if (val.type() == QVariant::String) {

            dev.write(indentString);
            writeString(dev, val, name);
            dev.write((newLine).toLatin1());
        } else if (val.type() == QVariant::Bool) {
            QString boolVal = val.toBool()? "true": "false";
            dev.write(indentString);
            dev.write((name+" "+boolVal+newLine).toLatin1());
        } else {
            if (val.type() == QVariant::Double) {
                dev.write(indentString);
                dev.write((name+" "+QString::number(val.toDouble(), 'f', 5)+newLine).toLatin1());
            } else if (val.type() == QVariant::Int) {
                dev.write(indentString);
                dev.write((name+" "+QString::number(val.toInt())+newLine).toLatin1());
            }
        }
    }
    dev.write(indentStringOld);
    dev.write((">>"+newLine).toLatin1());
}

QByteArray KisCosWriter::writeCosFromVariantHash(const QVariantHash doc)
{
    QByteArray ba;
    QBuffer dev(&ba);
    if (dev.open(QIODevice::WriteOnly)) {
        int indent = 0;
        dev.write("\n\n");
        bool prettyPrint = true;
        writeVariant(dev, doc, indent, prettyPrint);
        dev.close();
    } else {
        qDebug() << dev.errorString();
    }
    return ba;
}
