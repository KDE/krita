/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>
   SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
   SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kxmlguiversionhandler_p.h"

#include "kxmlguifactory.h"

#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QStandardPaths>
#include <QMap>

struct DocStruct {
    QString file;
    QString data;
};

static QList<QDomElement> extractToolBars(const QDomDocument &doc)
{
    QList<QDomElement> toolbars;
    QDomElement parent = doc.documentElement();
    for (QDomElement e = parent.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (e.tagName().compare(QStringLiteral("ToolBar"), Qt::CaseInsensitive) == 0) {
            toolbars.append(e);
        }
    }
    return toolbars;
}

static void removeAllToolBars(QDomDocument &doc)
{
    QDomElement parent = doc.documentElement();
    const QList<QDomElement> toolBars = extractToolBars(doc);
    Q_FOREACH (const QDomElement &e, toolBars) {
        parent.removeChild(e);
    }
}

static void insertToolBars(QDomDocument &doc, const QList<QDomElement> &toolBars)
{
    QDomElement parent = doc.documentElement();
    QDomElement menuBar = parent.namedItem(QStringLiteral("MenuBar")).toElement();
    QDomElement insertAfter = menuBar;
    if (menuBar.isNull()) {
        insertAfter = parent.firstChildElement();    // if null, insertAfter will do an append
    }
    Q_FOREACH (const QDomElement &e, toolBars) {
        QDomNode result = parent.insertAfter(e, insertAfter);
        Q_ASSERT(!result.isNull());
    }
}

//

typedef QMap<QString, QMap<QString, QString> > ActionPropertiesMap;

static ActionPropertiesMap extractActionProperties(const QDomDocument &doc)
{
    ActionPropertiesMap properties;

    QDomElement actionPropElement = doc.documentElement().namedItem(QStringLiteral("ActionProperties")).toElement();

    if (actionPropElement.isNull()) {
        return properties;
    }

    QDomNode n = actionPropElement.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        n = n.nextSibling(); // Advance now so that we can safely delete e
        if (e.isNull()) {
            continue;
        }

        if (e.tagName().compare(QStringLiteral("action"), Qt::CaseInsensitive) != 0) {
            continue;
        }

        const QString actionName = e.attribute(QStringLiteral("name"));
        if (actionName.isEmpty()) {
            continue;
        }

        QMap<QString, QMap<QString, QString> >::Iterator propIt = properties.find(actionName);
        if (propIt == properties.end()) {
            propIt = properties.insert(actionName, QMap<QString, QString>());
        }

        const QDomNamedNodeMap attributes = e.attributes();
        const uint attributeslength = attributes.length();

        for (uint i = 0; i < attributeslength; ++i) {
            const QDomAttr attr = attributes.item(i).toAttr();

            if (attr.isNull()) {
                continue;
            }

            const QString name = attr.name();

            if (name == QStringLiteral("name") || name.isEmpty()) {
                continue;
            }

            (*propIt)[ name ] = attr.value();
        }

    }

    return properties;
}

static void storeActionProperties(QDomDocument &doc,
                                  const ActionPropertiesMap &properties)
{
    QDomElement actionPropElement = doc.documentElement().namedItem(QStringLiteral("ActionProperties")).toElement();

    if (actionPropElement.isNull()) {
        actionPropElement = doc.createElement(QStringLiteral("ActionProperties"));
        doc.documentElement().appendChild(actionPropElement);
    }

//Remove only those ActionProperties entries from the document, that are present
//in the properties argument. In real life this means that local ActionProperties
//takes precedence over global ones, if they exists (think local override of shortcuts).
    QDomNode actionNode = actionPropElement.firstChild();
    while (!actionNode.isNull()) {
        if (properties.contains(actionNode.toElement().attribute(QStringLiteral("name")))) {
            QDomNode nextNode = actionNode.nextSibling();
            actionPropElement.removeChild(actionNode);
            actionNode = nextNode;
        } else {
            actionNode = actionNode.nextSibling();
        }
    }

    ActionPropertiesMap::ConstIterator it = properties.begin();
    const ActionPropertiesMap::ConstIterator end = properties.end();
    for (; it != end; ++it) {
        QDomElement action = doc.createElement(QStringLiteral("Action"));
        action.setAttribute(QStringLiteral("name"), it.key());
        actionPropElement.appendChild(action);

        const QMap<QString, QString> attributes = (*it);
        QMap<QString, QString>::ConstIterator attrIt = attributes.begin();
        const QMap<QString, QString>::ConstIterator attrEnd = attributes.end();
        for (; attrIt != attrEnd; ++attrIt) {
            action.setAttribute(attrIt.key(), attrIt.value());
        }
    }
}

QString KXmlGuiVersionHandler::findVersionNumber(const QString &xml)
{
    enum { ST_START, ST_AFTER_OPEN, ST_AFTER_GUI,
           ST_EXPECT_VERSION, ST_VERSION_NUM
         } state = ST_START;
    const int length = xml.length();
    for (int pos = 0; pos < length; pos++) {
        switch (state) {
        case ST_START:
            if (xml[pos] == QLatin1Char('<')) {
                state = ST_AFTER_OPEN;
            }
            break;
        case ST_AFTER_OPEN: {
            //Jump to gui..
            const int guipos = xml.indexOf(QStringLiteral("gui"), pos, Qt::CaseInsensitive);
            if (guipos == -1) {
                return QString();    //Reject
            }

            pos = guipos + 2; //Position at i, so we're moved ahead to the next character by the ++;
            state = ST_AFTER_GUI;
            break;
        }
        case ST_AFTER_GUI:
            state = ST_EXPECT_VERSION;
            break;
        case ST_EXPECT_VERSION: {
            const int verpos = xml.indexOf(QStringLiteral("version"), pos, Qt::CaseInsensitive);
            if (verpos == -1) {
                return QString();    //Reject
            }
            pos = verpos + 7; // strlen("version") is 7
            while (xml.at(pos).isSpace()) {
                ++pos;
            }
            if (xml.at(pos++) != QLatin1Char('=')) {
                return QString();    //Reject
            }
            while (xml.at(pos).isSpace()) {
                ++pos;
            }

            state = ST_VERSION_NUM;
            break;
        }
        case ST_VERSION_NUM: {
            int endpos;
            for (endpos = pos; endpos < length; endpos++) {
                const ushort ch = xml[endpos].unicode();
                if (ch >= QLatin1Char('0') && ch <= QLatin1Char('9')) {
                    continue;    //Number..
                }
                if (ch == QLatin1Char('"')) { //End of parameter
                    break;
                } else { //This shouldn't be here..
                    endpos = length;
                }
            }

            if (endpos != pos && endpos < length) {
                const QString matchCandidate = xml.mid(pos, endpos - pos); //Don't include " ".
                return matchCandidate;
            }

            state = ST_EXPECT_VERSION; //Try to match a well-formed version..
            break;
        } //case..
        } //switch
    } //for

    return QString();
}

KXmlGuiVersionHandler::KXmlGuiVersionHandler(const QStringList &files)
{
    Q_ASSERT(!files.isEmpty());

    if (files.count() == 1) {
        // No need to parse version numbers if there's only one file anyway
        m_file = files.first();
        m_doc = KXMLGUIFactory::readConfigFile(m_file);
        return;
    }

    QList<DocStruct> allDocuments;

    Q_FOREACH (const QString &file, files) {
        DocStruct d;
        d.file = file;
        d.data = KXMLGUIFactory::readConfigFile(file);
        allDocuments.append(d);
    }

    QList<DocStruct>::iterator best = allDocuments.end();
    uint bestVersion = 0;

    QList<DocStruct>::iterator docIt = allDocuments.begin();
    const QList<DocStruct>::iterator docEnd = allDocuments.end();
    for (; docIt != docEnd; ++docIt) {
        const QString versionStr = findVersionNumber((*docIt).data);
        if (versionStr.isEmpty()) {
            //qDebug(260) << "found no version in" << (*docIt).file;
            continue;
        }

        bool ok = false;
        uint version = versionStr.toUInt(&ok);
        if (!ok) {
            continue;
        }
        //qDebug(260) << "found version" << version << "for" << (*docIt).file;

        if (version > bestVersion) {
            best = docIt;
            //qDebug(260) << "best version is now " << version;
            bestVersion = version;
        }
    }

    if (best != docEnd) {
        if (best != allDocuments.begin()) {
            QList<DocStruct>::iterator local = allDocuments.begin();

            if ((*local).file.startsWith(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))) {
                // load the local document and extract the action properties
                QDomDocument localDocument;
                localDocument.setContent((*local).data);

                const ActionPropertiesMap properties = extractActionProperties(localDocument);
                const QList<QDomElement> toolbars = extractToolBars(localDocument);

                // in case the document has a ActionProperties section
                // we must not delete it but copy over the global doc
                // to the local and insert the ActionProperties section

                // TODO: kedittoolbar should mark toolbars as modified so that
                // we don't keep old toolbars just because the user defined a shortcut

                if (!properties.isEmpty() || !toolbars.isEmpty()) {
                    // now load the global one with the higher version number
                    // into memory
                    QDomDocument document;
                    document.setContent((*best).data);
                    // and store the properties in there
                    storeActionProperties(document, properties);
                    if (!toolbars.isEmpty()) {
                        // remove application toolbars
                        removeAllToolBars(document);
                        // add user toolbars
                        insertToolBars(document, toolbars);
                    }

                    (*local).data = document.toString();
                    // make sure we pick up the new local doc, when we return later
                    best = local;

                    // write out the new version of the local document
                    QFile f((*local).file);
                    if (f.open(QIODevice::WriteOnly)) {
                        const QByteArray utf8data = (*local).data.toUtf8();
                        f.write(utf8data.constData(), utf8data.length());
                        f.close();
                    }
                } else {
                    // Move away the outdated local file, to speed things up next time
                    const QString f = (*local).file;
                    const QString backup = f + QStringLiteral(".backup");
                    QFile::rename(f, backup);
                }
            }
        }
        m_doc = (*best).data;
        m_file = (*best).file;
    } else {
        //qDebug(260) << "returning first one...";
        m_doc = allDocuments.first().data;
        m_file = allDocuments.first().file;
    }
}
