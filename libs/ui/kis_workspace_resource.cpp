/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_workspace_resource.h"

#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QBuffer>


#define WORKSPACE_VERSION 1

KisWorkspaceResource::KisWorkspaceResource(const QString& filename): KoResource(filename)
{
}

KisWorkspaceResource::~KisWorkspaceResource()
{
}

KisWorkspaceResource::KisWorkspaceResource(const KisWorkspaceResource &rhs)
    : KoResource(rhs)
    , KisPropertiesConfiguration(rhs)
    , m_dockerState(rhs.m_dockerState)
{
}

KoResourceSP KisWorkspaceResource::clone() const
{
    return KoResourceSP(new KisWorkspaceResource(*this));
}

bool KisWorkspaceResource::saveToDevice(QIODevice *dev) const
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Workspace");
    root.setAttribute("name", name() );
    root.setAttribute("version", WORKSPACE_VERSION);
    QDomElement state = doc.createElement("state");
    state.appendChild(doc.createCDATASection(m_dockerState.toBase64()));
    root.appendChild(state);

    // Save KisPropertiesConfiguration settings
    QDomElement settings = doc.createElement("settings");
    KisPropertiesConfiguration::toXML(doc, settings);
    root.appendChild(settings);

    if (!image().isNull()) {
        QDomElement thumb = doc.createElement("image");
        QByteArray arr;
        QBuffer buffer(&arr);
        buffer.open(QIODevice::WriteOnly);
        image().save(&buffer, "PNG");
        buffer.close();
        thumb.appendChild(doc.createCDATASection(arr.toBase64()));
        root.appendChild(thumb);
    }

    doc.appendChild(root);

    QTextStream textStream(dev);
    textStream.setCodec("UTF-8");
    doc.save(textStream, 4);

    return true;

}

bool KisWorkspaceResource::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

    QDomDocument doc;
    if (!doc.setContent(dev)) {
        return false;
    }

    QDomElement element = doc.documentElement();
    setName(element.attribute("name"));

    QDomElement state = element.firstChildElement("state");

    if (!state.isNull()) {
        m_dockerState = QByteArray::fromBase64(state.text().toLatin1());
    }

    QDomElement settings = element.firstChildElement("settings");
    if (!settings.isNull()) {
        KisPropertiesConfiguration::fromXML(settings);
    }

    QDomElement thumb = element.firstChildElement("image");
    if (!thumb.isNull()) {
        QImage img;
        img.loadFromData(QByteArray::fromBase64(thumb.text().toLatin1()));
        this->setImage(img);
    }

    setValid(true);
    return true;
}

QString KisWorkspaceResource::defaultFileExtension() const
{
    return QString(".kws");
}

void KisWorkspaceResource::setDockerState(const QByteArray& state)
{
    m_dockerState = state;
}

QByteArray KisWorkspaceResource::dockerState()
{
    return m_dockerState;
}
