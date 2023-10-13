/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_cos_writer.h"

#include <QTextCodec>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QBuffer>

void writeJsonValue(QIODevice &dev, QJsonObject object, int indent) {
    QByteArray indentStringOld(indent, QChar::Tabulation);
    dev.write(indentStringOld);
    dev.write("<<\n");
    indent += 1;
    QByteArray indentString(indent, QChar::Tabulation);
    Q_FOREACH(QString key, object.keys()) {
        QJsonValue val = object[key];
        QString name = QString(key);
        if (val.isObject()) {
            dev.write(indentString);
            dev.write((name+"\n").toLatin1());
            writeJsonValue(dev, val.toObject(), indent);
        } else if (val.isArray()) {
            QJsonArray array = val.toArray();
            if (array.at(0).isDouble()) {
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
                for (int i=0; i<val.toArray().size(); i++) {
                    QJsonValue arrVal = val.toArray().at(i);
                    if (arrVal.isObject()) {
                        writeJsonValue(dev, arrVal.toObject(), indent);
                    }
                }
                dev.write(indentString);
                dev.write("]\n");
            }
        } else if (val.isString()) {
            QString newString = val.toString();
            newString.replace("\n", "\r");
            QTextCodec *Utf16Codec = QTextCodec::codecForName("UTF-16BE");
            dev.write(indentString);
            dev.write((name+" (").toLatin1());
            dev.write(Utf16Codec->fromUnicode(newString));
            dev.write(")\n");
        } else if (val.isBool()) {
            QString boolVal = val.toBool()? "true": "false";
            dev.write(indentString);
            dev.write((name+" "+boolVal+"\n").toLatin1());
        } else {
            dev.write(indentString);
            dev.write((name+" "+QString::number(val.toDouble(), 'f', 5)+"\n").toLatin1());
        }
    }
    dev.write(indentStringOld);
    dev.write(">>\n");
}

QByteArray KisCosWriter::writeCosFromJSON(QJsonDocument doc)
{
    QByteArray ba;
    QBuffer dev(&ba);
    if (dev.open(QIODevice::WriteOnly)) {
        int indent = 0;
        dev.write("\n\n");
        writeJsonValue(dev, doc.object(), indent);
        dev.close();
    } else {
        qDebug() << dev.errorString();
    }
    return ba;
}
