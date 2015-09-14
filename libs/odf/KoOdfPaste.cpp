/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoOdfPaste.h"

#include <QBuffer>
#include <QByteArray>
#include <QMimeData>
#include <QString>

#include <OdfDebug.h>

#include <KoStore.h>
#include <KoOdfReadStore.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>

KoOdfPaste::KoOdfPaste()
{
}

KoOdfPaste::~KoOdfPaste()
{
}

bool KoOdfPaste::paste(KoOdf::DocumentType documentType, const QMimeData *data)
{
    QByteArray arr = data->data(KoOdf::mimeType(documentType));
    return paste(documentType, arr);
}

bool KoOdfPaste::paste(KoOdf::DocumentType documentType, const QByteArray &bytes)
{
    if (bytes.isEmpty())
        return false;

    QBuffer buffer;
    buffer.setData(bytes);
    KoStore *store = KoStore::createStore(&buffer, KoStore::Read);
    //FIXME: Use shared_ptr or smth like these to auto delete store on return
    // and delete all next "delete store;".

    KoOdfReadStore odfStore(store); // KoOdfReadStore does not delete the store on destruction

    QString errorMessage;
    if (! odfStore.loadAndParse(errorMessage)) {
        warnOdf << "loading and parsing failed:" << errorMessage;
        delete store;
        return false;
    }

    KoXmlElement content = odfStore.contentDoc().documentElement();
    KoXmlElement realBody(KoXml::namedItemNS(content, KoXmlNS::office, "body"));

    if (realBody.isNull()) {
        warnOdf << "No body tag found";
        delete store;
        return false;
    }

    KoXmlElement body = KoXml::namedItemNS(realBody, KoXmlNS::office, KoOdf::bodyContentElement(documentType, false));

    if (body.isNull()) {
        warnOdf << "No" << KoOdf::bodyContentElement(documentType, true) << "tag found";
        delete store;
        return false;
    }

    bool retval = process(body, odfStore);
    delete store;
    return retval;
}
