/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "taskset_resource.h"

#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QBuffer>
#include <QByteArray>

#include <kis_debug.h>

#define TASKSET_VERSION 1

TasksetResource::TasksetResource(const QString& f)
    : KoResource(f)
{
}

TasksetResource::~TasksetResource()
{
}

bool TasksetResource::save()
{
    if (filename().isEmpty())
         return false;

    QFile file(filename());
    file.open(QIODevice::WriteOnly);
    bool res = saveToDevice(&file);
    file.close();
    return res;
}

bool TasksetResource::load()
{
    QString fn = filename();
    if (fn.isEmpty()) return false;
 
    QFile file(fn);
    if (file.size() == 0) return false;
    if (!file.open(QIODevice::ReadOnly)) {
        warnKrita << "Can't open file " << filename();
        return false;
    }

    bool res = loadFromDevice(&file);

    file.close();

    return res;
}

bool TasksetResource::loadFromDevice(QIODevice *dev)
{
    QDomDocument doc;
    if (!doc.setContent(dev)) {
        return false;
    }
    QDomElement element = doc.documentElement();
    setName(element.attribute("name"));
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement child = node.toElement();
        if (!child.isNull() && child.tagName() == "action") {
                m_actions.append(child.text());
        }
        node = node.nextSibling();
    }
    setValid(true);
    return true;
}

QString TasksetResource::defaultFileExtension() const
{
    return QString(".kts");
}

void TasksetResource::setActionList(const QStringList actions)
{
    m_actions = actions;
}

QStringList TasksetResource::actionList()
{
    return m_actions;
}

bool TasksetResource::saveToDevice(QIODevice *io) const
{

    QDomDocument doc;
    QDomElement root = doc.createElement("Taskset");
    root.setAttribute("name", name() );
    root.setAttribute("version", TASKSET_VERSION);
    Q_FOREACH (const QString& action, m_actions) {
        QDomElement element = doc.createElement("action");
        element.appendChild(doc.createTextNode(action));
        root.appendChild(element);
    }
    doc.appendChild(root);

    QTextStream textStream(io);
    textStream.setCodec("UTF-8");
    doc.save(textStream, 4);

    KoResource::saveToDevice(io);

    return true;
}


