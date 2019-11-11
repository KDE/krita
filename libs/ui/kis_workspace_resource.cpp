/* This file is part of the KDE project
 * Copyright (C) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_workspace_resource.h"

#include <QFile>
#include <QDomDocument>
#include <QTextStream>


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
{
    *this = rhs;
}

KisWorkspaceResource &KisWorkspaceResource::operator=(const KisWorkspaceResource &rhs)
{
    if (*this != rhs) {
        m_dockerState = rhs.m_dockerState;
    }
    return *this;
}

KoResourceSP KisWorkspaceResource::clone() const
{
    return KoResourceSP(new KisWorkspaceResource(*this));
}


bool KisWorkspaceResource::save()
{
    if (filename().isEmpty())
         return false;

    QFile file(filename());
    file.open(QIODevice::WriteOnly);
    bool res = saveToDevice(&file);
    file.close();
    return res;
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
    doc.appendChild(root);

    QTextStream textStream(dev);
    textStream.setCodec("UTF-8");
    doc.save(textStream, 4);

    KoResource::saveToDevice(dev);

    return true;

}

bool KisWorkspaceResource::load()
{
    if (filename().isEmpty())
         return false;
 
    QFile file(filename());
    if (file.size() == 0) return false;
    if (!file.open(QIODevice::ReadOnly)) {
        warnKrita << "Can't open file " << filename();
        return false;
    }

    bool res = loadFromDevice(&file);
    file.close();
    return res;
}

bool KisWorkspaceResource::loadFromDevice(QIODevice *dev)
{
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
