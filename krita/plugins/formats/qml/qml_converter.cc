/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "qml_converter.h"

#include <QFileInfo>
#include <QDir>
#include <kapplication.h>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <kis_image.h>
#include <kis_group_layer.h>

#define SPACE "    "

QMLConverter::QMLConverter()
{
}

QMLConverter::~QMLConverter()
{
}

KisImageBuilder_Result QMLConverter::buildFile(const KUrl& uri, KisImageWSP image)
{

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;

    QFile file(uri.path());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
         return KisImageBuilder_RESULT_FAILURE;
    }

    QTextStream out(&file);
    out << "import QtQuick 1.1" << "\n\n";
    out << "Rectangle {\n";
    writeInt(out, 1, "width", image->width());
    writeInt(out, 1, "height", image->height());
    out << "\n";

    QFileInfo info(file);
    KisNodeSP node = image->rootLayer()->firstChild();
    QString imageDir = info.baseName() + "_images";
    QString imagePath = info.absolutePath() + '/' + imageDir;
    if (node) {
        QDir dir;
        dir.mkpath(imagePath);
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
        out << SPACE << "}\n";
        node = node->nextSibling();
    }
    out << "}\n";


    file.close();

    return KisImageBuilder_RESULT_OK;
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

#include "qml_converter.moc"

