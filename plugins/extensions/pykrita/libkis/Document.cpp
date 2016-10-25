/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "Document.h"
#include <QPointer>

#include <KisDocument.h>

struct Document::Private {
    Private() {}
    QPointer<KisDocument> document;
};

Document::Document(KisDocument *document, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->document = document;
}

Document::~Document()
{
    delete d;
}

Node* Document::activeNode() const
{
    return 0;
}

void Document::setActiveNode(Node* value)
{
}


ColorDepth* Document::colorDepth() const
{
    return 0;
}

void Document::setColorDepth(ColorDepth* value)
{
}


ColorManager* Document::colorManager() const
{
    return 0;
}

void Document::setColorManager(ColorManager* value)
{
}


ColorModel* Document::colorModel() const
{
    return 0;
}

void Document::setColorModel(ColorModel* value)
{
}


ColorProfile* Document::colorProfile() const
{
    return 0;
}

void Document::setColorProfile(ColorProfile* value)
{
}


InfoObject* Document::documentInfo() const
{
    return 0;
}

void Document::setDocumentInfo(InfoObject* value)
{
}


QString Document::fileName() const
{
    return QString();
}

void Document::setFileName(QString value)
{
}


int Document::height() const
{
    return 0;
}

void Document::setHeight(int value)
{
}


InfoObject* Document::metaData() const
{
    return 0;
}

void Document::setMetaData(InfoObject* value)
{
}


QString Document::name() const
{
    return QString();
}

void Document::setName(QString value)
{
}


int Document::resolution() const
{
    return 0;
}

void Document::setResolution(int value)
{
}


Node* Document::rootNode() const
{
    return 0;
}

void Document::setRootNode(Node* value)
{
}


Selection* Document::selection() const
{
    return 0;
}

void Document::setSelection(Selection* value)
{
}


int Document::width() const
{
    return 0;
}

void Document::setWidth(int value)
{
}


QByteArray Document::pixelData() const
{
    return QByteArray();
}

void Document::setPixelData(QByteArray value)
{
}




Document * Document::clone()
{
    return 0;
}

bool Document::close()
{
    return false;
}

bool Document::convert(const QString &colorModel, const ColorProfile *profile)
{
    return false;
}

void Document::crop(int x, int y, int w, int h)
{
}

bool Document::Export(const InfoObject &exportConfiguration)
{
    return false;
}

void Document::Flatten()
{
}

void Document::ResizeImage(int w, int h)
{
}

bool Document::Save(const QString &url)
{
    return false;
}

bool Document::SaveAs(const QString &url)
{
    return false;
}

void Document::OpenView()
{
}

Node* Document::CreateNode(const QString &name, const QString &nodeType)
{
    return 0;
}



