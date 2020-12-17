/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "qml_converter.h"

#include <QFileInfo>
#include <QDir>

#include <kis_image.h>
#include <kis_group_layer.h>

#define SPACE "    "

QMLConverter::QMLConverter()
{
}

QMLConverter::~QMLConverter()
{
}

KisImportExportErrorCode QMLConverter::buildFile(const QString &filename, const QString &realFilename, QIODevice *io, KisImageSP image)
{
    QTextStream out(io);
    out.setCodec("UTF-8");
    out << "import QtQuick 1.1" << "\n\n";
    out << "Rectangle {\n";
    writeInt(out, 1, "width", image->width());
    writeInt(out, 1, "height", image->height());
    out << "\n";

    QFileInfo info(filename);
    QFileInfo infoRealFile(realFilename);
    KisNodeSP node = image->rootLayer()->firstChild();
    QString imageDir = infoRealFile.completeBaseName() + "_images";
    QString imagePath = infoRealFile.absolutePath() + '/' + imageDir;
    if (node) {
        QDir dir;
        bool success = dir.mkpath(imagePath);
        if (!success)
        {
            return ImportExportCodes::CannotCreateFile;
        }
    }

    dbgFile << "Saving images to " << imagePath;
    while(node) {
        KisPaintDeviceSP projection = node->projection();
        QRect rect = projection->exactBounds();
        QImage qmlImage = projection->convertToQImage(0, rect.x(), rect.y(), rect.width(), rect.height());
        QString name = node->name().replace(' ', '_').toLower();
        QString fileName = name + ".png";
        qmlImage.save(imagePath +'/'+ fileName);

        out << SPACE << "Image {\n";
        writeString(out, 2, "id", name);
        writeInt(out, 2, "x", rect.x());
        writeInt(out, 2, "y", rect.y());
        writeInt(out, 2, "width", rect.width());
        writeInt(out, 2, "height", rect.height());
        writeString(out, 2, "source", "\"" + imageDir + '/' + fileName + "\"" );
        writeString(out, 2, "opacity", QString().setNum(node->opacity()/255.0));
        out << SPACE << "}\n";
        node = node->nextSibling();
    }
    out << "}\n";

    return ImportExportCodes::OK;
}

void QMLConverter::writeString(QTextStream&  out, int spacing, const QString& setting, const QString& value) {
    for (int space = 0; space < spacing; space++) {
        out << SPACE;
    }
    out << setting << ": " << value << "\n";
}

void QMLConverter::writeInt(QTextStream&  out, int spacing, const QString& setting, int value) {
    writeString(out, spacing, setting, QString::number(value));
}


