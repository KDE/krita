/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_asl_xml_parser.h"

#include <boost/function.hpp>

#include <stdexcept>
#include <string>

#include <QDomDocument>
#include <QIODevice>
#include <QBuffer>

#include <QColor>
#include <QHash>

#include "kis_dom_utils.h"

#include "kis_debug.h"
#include "psd_utils.h"
#include "psd.h"
#include "compression.h"


#include "kis_asl_object_catcher.h"

namespace Private {

void parseElement(const QDomElement &el,
                  const QString &parentPath,
                  KisAslObjectCatcher &catcher);

class CurveObjectCatcher : public KisAslObjectCatcher
{
public:
    void addDouble(const QString &path, double value) {
        qWarning() << "XML (ASL): failed to parse curve object" << path << value;
    }

    void addInteger(const QString &path, int value) {
        qWarning() << "XML (ASL): failed to parse curve object" << path << value;
    }

    void addEnum(const QString &path, const QString &typeId, const QString &value) {
        Q_UNUSED(typeId);
        qWarning() << "XML (ASL): failed to parse curve object" << path << value;
    }

    void addUnitFloat(const QString &path, const QString &unit, double value) {
        Q_UNUSED(unit);
        qWarning() << "XML (ASL): failed to parse curve object" << path << value;
    }

    void addText(const QString &path, const QString &value) {
        if (path == "/Nm  ") {
            m_name = value;
        } else {
            qWarning() << "XML (ASL): failed to parse curve object" << path << value;
        }
    }

    void addBoolean(const QString &path, bool value) {
        qWarning() << "XML (ASL): failed to parse curve object" << path << value;
    }

    void addColor(const QString &path, const QColor &value) {
        qWarning() << "XML (ASL): failed to parse curve object" << path << value;
    }

    void addPoint(const QString &path, const QPointF &value) {
        if (!m_arrayMode) {
            qWarning() << "XML (ASL): failed to parse curve object (array fault)" << path << value << ppVar(m_arrayMode);
        }

        m_points.append(value);
    }

    void addCurve(const QString &path, const QString &name, const QVector<QPointF> &points) {
        Q_UNUSED(points);
        qWarning() << "XML (ASL): failed to parse curve object" << path << name;
    }

    void addPattern(const QString &path, const KoPattern *pattern) {
        qWarning() << "XML (ASL): failed to parse curve object" << path << pattern;
    }

public:
    QVector<QPointF> m_points;
    QString m_name;
};

inline QString buildPath(const QString &parent, const QString &key) {
    return parent + "/" + key;
}

bool tryParseDescriptor(const QDomElement &el,
                        const QString &path,
                        const QString &classId,
                        KisAslObjectCatcher &catcher)
{
    bool retval = true;

    if (classId == "RGBC") {
        QColor color(Qt::black);

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            QDomElement childEl = child.toElement();

            QString type = childEl.attribute("type", "<unknown>");
            QString key = childEl.attribute("key", "");

            if (type != "Double") {
                qWarning() << "Unknown color component type:" << ppVar(type) << ppVar(key) << ppVar(path);
                return false;
            }

            double value = KisDomUtils::Private::stringToDouble(childEl.attribute("value", "0"));

            if (key == "Rd  ") {
                color.setRed(value);
            } else if (key == "Grn ") {
                color.setGreen(value);
            } else if (key == "Bl  ") {
                color.setBlue(value);
            } else {
                qWarning() << "Unknown color key value:" << ppVar(key) << ppVar(path);
                return false;
            }

            child = child.nextSibling();
        }

        catcher.addColor(path, color);

    } else if (classId == "ShpC") {
        CurveObjectCatcher curveCatcher;

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            parseElement(child.toElement(), "", curveCatcher);
            child = child.nextSibling();
        }

        catcher.addCurve(path, curveCatcher.m_name, curveCatcher.m_points);

    } else if (classId == "CrPt") {
        QPointF point;

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            QDomElement childEl = child.toElement();

            QString type = childEl.attribute("type", "<unknown>");
            QString key = childEl.attribute("key", "");


            if (type == "Boolean" && key == "Cnty") {
                qWarning() << "WARNING: tryParseDescriptor: The points of the curve object contain \'Cnty\' flag which is unsupported by Krita";
                qWarning() << "        " << ppVar(type) << ppVar(key) << ppVar(path);

                child = child.nextSibling();
                continue;
            }

            if (type != "Double") {
                qWarning() << "Unknown point component type:" << ppVar(type) << ppVar(key) << ppVar(path);
                return false;
            }

            double value = KisDomUtils::Private::stringToDouble(childEl.attribute("value", "0"));

            if (key == "Hrzn") {
                point.setX(value);
            } else if (key == "Vrtc") {
                point.setY(value);
            } else {
                qWarning() << "Unknown point key value:" << ppVar(key) << ppVar(path);
                return false;
            }

            child = child.nextSibling();
        }

        catcher.addPoint(path, point);
    } else if (classId == "KisPattern") {
        QByteArray patternData;
        QString patternUuid;

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            QDomElement childEl = child.toElement();

            QString type = childEl.attribute("type", "<unknown>");
            QString key = childEl.attribute("key", "");

            if (type == "Text" && key == "Idnt") {
                patternUuid = childEl.attribute("value", "");
            }

            if (type == "KisPatternData" && key == "Data") {
                QDomNode dataNode = child.firstChild();

                if (!dataNode.isCDATASection()) {
                    qWarning() << "WARNING: failed to parse KisPatternData XML section!";
                    continue;
                }

                QDomCDATASection dataSection = dataNode.toCDATASection();
                QByteArray data = dataSection.data().toAscii();
                data = QByteArray::fromBase64(data);
                data = qUncompress(data);

                if (data.isEmpty()) {
                    qWarning() << "WARNING: failed to parse KisPatternData XML section!";
                    continue;
                }

                patternData = data;
            }

            child = child.nextSibling();
        }

        if (!patternUuid.isEmpty() && !patternData.isEmpty()) {
            QString fileName = QString("%1.pat").arg(patternUuid);

            QScopedPointer<KoPattern> pattern(new KoPattern(fileName));

            QBuffer buffer(&patternData);
            buffer.open(QIODevice::ReadOnly);

            pattern->loadFromDevice(&buffer);

            catcher.addPattern(path, pattern.data());
        } else {
            qWarning() << "WARNING: failed to load KisPattern XML section!" << ppVar(patternUuid);
        }

    } else {
        retval = false;
    }

    return retval;
}

void parseElement(const QDomElement &el, const QString &parentPath, KisAslObjectCatcher &catcher)
{
    KIS_ASSERT_RECOVER_RETURN(el.tagName() == "node");

    QString type = el.attribute("type", "<unknown>");
    QString key = el.attribute("key", "");

    if (type == "Descriptor") {
        QString classId = el.attribute("classId", "<noClassId>");
        QString containerName = key.isEmpty() ? classId : key;
        QString containerPath = buildPath(parentPath, containerName);

        if (!tryParseDescriptor(el, containerPath, classId, catcher)) {
            QDomNode child = el.firstChild();
            while (!child.isNull()) {
                parseElement(child.toElement(), containerPath, catcher);
                child = child.nextSibling();
            }
        }
    } else if (type == "List") {
        catcher.setArrayMode(true);

        QString containerName = key;
        QString containerPath = buildPath(parentPath, containerName);

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            parseElement(child.toElement(), containerPath, catcher);
            child = child.nextSibling();
        }

        catcher.setArrayMode(false);
    } else if (type == "Double") {
        double v = KisDomUtils::Private::stringToDouble(el.attribute("value", "0"));
        catcher.addDouble(buildPath(parentPath, key), v);
    } else if (type == "UnitFloat") {
        QString unit = el.attribute("unit", "<unknown>");
        double v = KisDomUtils::Private::stringToDouble(el.attribute("value", "0"));
        catcher.addUnitFloat(buildPath(parentPath, key), unit, v);
    } else if (type == "Text") {
        QString v = el.attribute("value", "");
        catcher.addText(buildPath(parentPath, key), v);
    } else if (type == "Enum") {
        QString v = el.attribute("value", "");
        QString typeId = el.attribute("typeId", "<unknown>");
        catcher.addEnum(buildPath(parentPath, key), typeId, v);
    } else if (type == "Integer") {
        int v = KisDomUtils::Private::stringToInt(el.attribute("value", "0"));
        catcher.addInteger(buildPath(parentPath, key), v);
    } else if (type == "Boolean") {
        int v = KisDomUtils::Private::stringToInt(el.attribute("value", "0"));
        catcher.addBoolean(buildPath(parentPath, key), v);
    } else {
        qWarning() << "WARNING: XML (ASL) Unknown element type:" << type << ppVar(parentPath) << ppVar(key);
    }
}

} // namespace

void KisAslXmlParser::parseXML(const QDomDocument &doc, KisAslObjectCatcher &catcher)
{
    QDomElement root = doc.documentElement();
    if (root.tagName() != "asl") {
        return;
    }

    QDomNode child = root.firstChild();
    while (!child.isNull()) {
        Private::parseElement(child.toElement(), "", catcher);
        child = child.nextSibling();
    }
}
