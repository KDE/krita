/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zach3n <zachmann@kde.org>

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

#include "KoOdfReadStore.h"

#include <OdfDebug.h>
#include <klocalizedstring.h>

#include <KoStore.h>
#include <KoXmlReader.h>

#include "KoOdfStylesReader.h"

#include <QXmlStreamReader>

class Q_DECL_HIDDEN KoOdfReadStore::Private
{
public:
    Private(KoStore *s)
            : store(s)
    {
    }

    KoStore * store;
    KoOdfStylesReader stylesReader;
    // it is needed to keep the stylesDoc around so that you can access the styles
    KoXmlDocument stylesDoc;
    KoXmlDocument contentDoc;
    KoXmlDocument settingsDoc;
};

KoOdfReadStore::KoOdfReadStore(KoStore *store)
        : d(new Private(store))
{
}

KoOdfReadStore::~KoOdfReadStore()
{
    delete d;
}

KoStore * KoOdfReadStore::store() const
{
    return d->store;
}

KoOdfStylesReader &KoOdfReadStore::styles()
{
    return d->stylesReader;
}

KoXmlDocument KoOdfReadStore::contentDoc() const
{
    return d->contentDoc;
}

KoXmlDocument KoOdfReadStore::settingsDoc() const
{
    return d->settingsDoc;
}

bool KoOdfReadStore::loadAndParse(QString &errorMessage)
{
    if (!loadAndParse("content.xml", d->contentDoc, errorMessage)) {
        return false;
    }

    if (d->store->hasFile("styles.xml")) {
        if (!loadAndParse("styles.xml", d->stylesDoc, errorMessage)) {
            return false;
        }
    }
    // Load styles from style.xml
    d->stylesReader.createStyleMap(d->stylesDoc, true);
    // Also load styles from content.xml
    d->stylesReader.createStyleMap(d->contentDoc, false);

    if (d->store->hasFile("settings.xml")) {
        loadAndParse("settings.xml", d->settingsDoc, errorMessage);
    }
    return true;
}

bool KoOdfReadStore::loadAndParse(const QString &fileName, KoXmlDocument &doc, QString &errorMessage)
{
    if (!d->store) {
        errorMessage = i18n("No store backend");
        return false;
    }

    if (!d->store->isOpen()) {
        if (!d->store->open(fileName)) {
            debugOdf << "Entry " << fileName << " not found!"; // not a warning as embedded stores don't have to have all files
            errorMessage = i18n("Could not find %1", fileName);
            return false;
        }
    }

    bool ok = loadAndParse(d->store->device(), doc, errorMessage, fileName);
    d->store->close();
    return ok;
}

bool KoOdfReadStore::loadAndParse(QIODevice *fileDevice, KoXmlDocument &doc, QString &errorMessage, const QString &fileName)
{
    // Error variables for QDomDocument::setContent
    QString errorMsg;
    int errorLine, errorColumn;

    if (!fileDevice->isOpen()) {
        fileDevice->open(QIODevice::ReadOnly);
    }

    QXmlStreamReader reader(fileDevice);
    reader.setNamespaceProcessing(true);

    bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
    if (!ok) {
        errorOdf << "Parsing error in " << fileName << "! Aborting!" << endl
        << " In line: " << errorLine << ", column: " << errorColumn << endl
        << " Error message: " << errorMsg << endl;
        errorMessage = i18n("Parsing error in the main document at line %1, column %2\nError message: %3"
                            , errorLine , errorColumn , errorMsg);
    } else {
        debugOdf << "File" << fileName << " loaded and parsed";
    }
    return ok;
}
