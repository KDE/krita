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

void writeJsonValue(QIODevice &dev, QVariantHash object, int indent) {
    QByteArray indentStringOld(indent, QChar::Tabulation);
    dev.write(indentStringOld);
    dev.write("<<\n");
    indent += 1;
    QByteArray indentString(indent, QChar::Tabulation);
    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object[key];
        QString name = QString(key);
        if (val.type() == QVariant::Hash) {
            dev.write(indentString);
            dev.write((name+"\n").toLatin1());
            writeJsonValue(dev, val.toHash(), indent);
        } else if (val.type() == QVariant::List) {
            QVariantList array = val.toList();
            if (array.at(0).type() == QVariant::Int) {
                dev.write(indentString);
                dev.write((name+" [").toLatin1());
                for (int i=0; i<array.size(); i++) {
                    dev.write((" "+QString::number(array.at(i).toInt())).toLatin1());
                }
                dev.write(" ]\n");
            } else if (array.at(0).type() == QVariant::Double) {
                dev.write(indentString);
                dev.write((name+" [").toLatin1());
                for (int i=0; i<array.size(); i++) {
                    dev.write((" "+QString::number(array.at(i).toDouble(), 'f', 5)).toLatin1());
                }
                dev.write(" ]\n");
            } else if(array.isEmpty()) {
                dev.write(indentString);
                dev.write((name+" [ ]\n").toLatin1());
            } else {
                dev.write(indentString);
                dev.write((name+" [\n").toLatin1());
                for (int i=0; i<val.toList().size(); i++) {
                    QVariant arrVal = val.toList().at(i);
                    if (arrVal.type() == QVariant::Hash) {
                        writeJsonValue(dev, arrVal.toHash(), indent);
                    }
                }
                dev.write(indentString);
                dev.write("]\n");
            }
        } else if (val.type() == QVariant::String) {
            QString newString = val.toString();
            newString.replace("\n", "\r");
            QTextCodec *Utf16Codec = QTextCodec::codecForName("UTF-16BE");
            dev.write(indentString);
            dev.write((name+" (").toLatin1());
            dev.write(Utf16Codec->fromUnicode(newString));
            dev.write(")\n");
        } else if (val.type() == QVariant::Bool) {
            QString boolVal = val.toBool()? "true": "false";
            dev.write(indentString);
            dev.write((name+" "+boolVal+"\n").toLatin1());
        } else {
            if (val.type() == QVariant::Double) {
                dev.write(indentString);
                dev.write((name+" "+QString::number(val.toDouble(), 'f', 5)+"\n").toLatin1());
            } else if (val.type() == QVariant::Int) {
                dev.write(indentString);
                dev.write((name+" "+QString::number(val.toInt())+"\n").toLatin1());
            }
        }
    }
    dev.write(indentStringOld);
    dev.write(">>\n");
}

QByteArray KisCosWriter::writeCosFromVariantHash(QVariantHash doc)
{
    QByteArray ba;
    QBuffer dev(&ba);
    if (dev.open(QIODevice::WriteOnly)) {
        int indent = 0;
        dev.write("\n\n");
        writeJsonValue(dev, doc, indent);
        dev.close();
    } else {
        qDebug() << dev.errorString();
    }
    return ba;
}
