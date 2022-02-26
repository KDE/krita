/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_asl_xml_parser.h"

#include <stdexcept>
#include <string>

#include <QBuffer>
#include <QDomDocument>
#include <QIODevice>
#include <QFileInfo>

#include <QColor>
#include <QHash>

#include <KoColorSpaceRegistry.h>
#include <KoColorConversions.h>
#include <resources/KoSegmentGradient.h>

#include "kis_dom_utils.h"

#include "compression.h"
#include "kis_debug.h"
#include "psd.h"
#include "psd_utils.h"

#include "kis_asl_object_catcher.h"

namespace Private
{
void parseElement(const QDomElement &el, const QString &parentPath, KisAslObjectCatcher &catcher);

class CurveObjectCatcher : public KisAslObjectCatcher
{
public:
    void addText(const QString &path, const QString &value) override
    {
        if (path == "/Nm  ") {
            m_name = value;
        } else {
            warnKrita << "XML (ASL): failed to parse curve object" << path << value;
        }
    }

    void addPoint(const QString &path, const QPointF &value) override
    {
        if (!m_arrayMode) {
            warnKrita << "XML (ASL): failed to parse curve object (array fault)" << path << value << ppVar(m_arrayMode);
        }

        m_points.append(value);
    }

public:
    QVector<QPointF> m_points;
    QString m_name;
};

KoColor parseColorObject(QDomElement parent, QString classID)
{
    KoColor color;
    KoColor error = KoColor::fromXML("<color channeldepth='U8'><sRGB r='1.0' g='0.0' b='0.0'/></color>");
    QDomDocument doc;
    QDomElement root;
    QString spotBook;
    QString spotName;
    int spotValue = 0;
    double h = 0;
    double s = 0;
    double v= 0;

    if (classID == "RGBC" || classID == "HSBC") {
        color = KoColor(KoColorSpaceRegistry::instance()->rgb8());
        root = doc.createElement("sRGB");
    } else if (classID == "CMYC") {
        root = doc.createElement("CMYK");
    } else if (classID == "LbCl") {
        root = doc.createElement("Lab");
    } else if (classID == "Grsc") {
        root = doc.createElement("Gray");
    } else {
        // Can be 'UnsC', or something else.
        warnKrita << "Unknown color type:" << ppVar(classID);
        return error;
    }

    QDomNode child = parent.firstChild();
    while (!child.isNull()) {
        QDomElement childEl = child.toElement();

        QString type = childEl.attribute("type", "<unknown>");
        QString key = childEl.attribute("key", "");

        if (type == "Double" || type == "UnitFloat") {
            if (classID == "RGBC") {
                // For RGBC we'll just directly write to the KoColor data, to have as
                // few rounding errors possible.
                double value = KisDomUtils::toDouble(childEl.attribute("value", "0"));

                if (key == "Rd  ") {
                    color.data()[2] = value;
                } else if (key == "Grn ") {
                    color.data()[1] = value;
                } else if (key == "Bl  ") {
                    color.data()[0] = value;
                } else {
                    warnKrita << "Unknown color key value double:" << ppVar(key);
                    return error;
                }
            } else if (classID == "CMYC") {
                double value = KisDomUtils::toDouble(childEl.attribute("value", "0")) * 0.01;
                // CMYK is stored in percentages...
                if (key == "Cyn ") {
                    root.setAttribute("c", value);
                } else if (key == "Mgnt") {
                    root.setAttribute("m", value);
                } else if (key == "Ylw ") {
                    root.setAttribute("y", value);
                } else if (key == "Blck") {
                    root.setAttribute("k", value);
                } else {
                    warnKrita << "Unknown color key value double:" << ppVar(key);
                    return error;
                }
            } else if (classID == "LbCl") {
                if (key == "Lmnc") {
                    root.setAttribute("L", childEl.attribute("value", "0"));
                } else if (key == "A   ") {
                    root.setAttribute("a", childEl.attribute("value", "0"));
                } else if (key == "B   ") {
                    root.setAttribute("b", childEl.attribute("value", "0"));
                } else {
                    warnKrita << "Unknown color key value:" << ppVar(key);
                    return error;
                }
            } else if (classID == "Grsc") {
                // Unsure that grey is stored as a percentage, might also be 255.
                double value = KisDomUtils::toDouble(childEl.attribute("value", "0")) * 0.01;
                if (key == "Gry ") {
                    root.setAttribute("g", value);
                } else {
                    warnKrita << "Unknown color key value:" << ppVar(key);
                    return error;
                }
            } else if (classID == "HSBC") {
                double value = KisDomUtils::toDouble(childEl.attribute("value", "0"));
                if (key == "H   ") {
                    h = value;
                } else if (key == "Strt") {
                    s = value * 0.01;
                } else if (key == "Brgh") {
                    v = value * 0.01;
                } else {
                    warnKrita << "Unknown color key value:" << ppVar(key);
                    return error;
                }
            }
        } else if (type == "Text") {
            if (key== "Bk  ") {
                spotBook = childEl.attribute("value", "");
            } else if (key== "Nm  ") {
                spotName = childEl.attribute("value", "");
            } else {
                warnKrita << "Unknown color key value string:" << ppVar(key);
            }
        } else if (type == "Integer") {
            if (key== "bookID") {
                spotValue = KisDomUtils::toInt(childEl.attribute("value", "0"));
            } else {
                warnKrita << "Unknown color key value integer:" << ppVar(key);
            }
        } else {
            qDebug() << "Unknown color component type:" << ppVar(type) << ppVar(key);
            return error;
        }

        child = child.nextSibling();
    }
    if (classID == "HSBC") {
        float r = 0.0;
        float b = 0.0;
        float g = 0.0;
        HSVToRGB(h, s, v, &r, &g, &b);
        root.setAttribute("r", r);
        root.setAttribute("g", g);
        root.setAttribute("b", b);
    }
    if (classID != "RGBC") {
        color = KoColor::fromXML(root, "U8");
    }
    color.setOpacity(OPACITY_OPAQUE_U8);
    if (!spotName.isEmpty()) {
        color.addMetadata("spotName", spotName);
        color.addMetadata("psdSpotBook", spotBook);
        color.addMetadata("psdSpotBookId", spotValue);
    }

    return color;
}

void parseColorStopsList(QDomElement parent,
                         QVector<qreal> &startLocations,
                         QVector<qreal> &middleOffsets,
                         QVector<KoColor> &colors,
                         QVector<KoGradientSegmentEndpointType> &types)
{
    QDomNode child = parent.firstChild();
    while (!child.isNull()) {
        QDomElement childEl = child.toElement();

        QString type = childEl.attribute("type", "<unknown>");
        QString key = childEl.attribute("key", "");
        QString classId = childEl.attribute("classId", "");

        if (type == "Descriptor" && classId == "Clrt") {
            // sorry for naming...
            QDomNode child = childEl.firstChild();
            while (!child.isNull()) {
                QDomElement childEl = child.toElement();

                QString type = childEl.attribute("type", "<unknown>");
                QString key = childEl.attribute("key", "");
                QString classId = childEl.attribute("classId", "");

                if (type == "Integer" && key == "Lctn") {
                    int value = KisDomUtils::toInt(childEl.attribute("value", "0"));
                    startLocations.append(qreal(value) / 4096.0);

                } else if (type == "Integer" && key == "Mdpn") {
                    int value = KisDomUtils::toInt(childEl.attribute("value", "0"));
                    middleOffsets.append(qreal(value) / 100.0);

                } else if (type == "Descriptor" && key == "Clr ") {
                    colors.append(parseColorObject(childEl, classId));

                } else if (type == "Enum" && key == "Type") {
                    QString typeId = childEl.attribute("typeId", "");

                    if (typeId != "Clry") {
                        warnKrita << "WARNING: Invalid typeId of a gradient stop type" << typeId;
                    }

                    QString value = childEl.attribute("value", "");
                    if (value == "BckC") {
                        types.append(BACKGROUND_ENDPOINT);
                    } else if (value == "FrgC") {
                        types.append(FOREGROUND_ENDPOINT);
                    } else {
                        types.append(COLOR_ENDPOINT);
                    }
                }

                child = child.nextSibling();
            }
        } else {
            warnKrita << "WARNING: Unrecognized object in color stops list" << ppVar(type) << ppVar(key) << ppVar(classId);
        }

        child = child.nextSibling();
    }
}

void parseTransparencyStopsList(QDomElement parent, QVector<qreal> &startLocations, QVector<qreal> &middleOffsets, QVector<qreal> &transparencies)
{
    QDomNode child = parent.firstChild();
    while (!child.isNull()) {
        QDomElement childEl = child.toElement();

        QString type = childEl.attribute("type", "<unknown>");
        QString key = childEl.attribute("key", "");
        QString classId = childEl.attribute("classId", "");

        if (type == "Descriptor" && classId == "TrnS") {
            // sorry for naming again...
            QDomNode child = childEl.firstChild();
            while (!child.isNull()) {
                QDomElement childEl = child.toElement();

                QString type = childEl.attribute("type", "<unknown>");
                QString key = childEl.attribute("key", "");

                if (type == "Integer" && key == "Lctn") {
                    int value = KisDomUtils::toInt(childEl.attribute("value", "0"));
                    startLocations.append(qreal(value) / 4096.0);
                } else if (type == "Integer" && key == "Mdpn") {
                    int value = KisDomUtils::toInt(childEl.attribute("value", "0"));
                    middleOffsets.append(qreal(value) / 100.0);
                } else if (type == "UnitFloat" && key == "Opct") {
                    QString unit = childEl.attribute("unit", "");
                    if (unit != "#Prc") {
                        warnKrita << "WARNING: Invalid unit of a gradient stop transparency" << unit;
                    }

                    qreal value = KisDomUtils::toDouble(childEl.attribute("value", "100"));
                    transparencies.append(value / 100.0);
                }

                child = child.nextSibling();
            }

        } else {
            warnKrita << "WARNING: Unrecognized object in transparency stops list" << ppVar(type) << ppVar(key) << ppVar(classId);
        }

        child = child.nextSibling();
    }
}

inline QString buildPath(const QString &parent, const QString &key)
{
    return parent + "/" + key;
}

bool tryParseDescriptor(const QDomElement &el, const QString &path, const QString &classId, KisAslObjectCatcher &catcher)
{
    bool retval = true;

    if (classId == "null") {
        catcher.newStyleStarted();
        // here we just notify that a new style is started, we haven't
        // processed the whole block yet, so return false.
        retval = false;
    } else if (el.attribute("key", " ") == "Clr ") {
        catcher.addColor(path, parseColorObject(el, classId));
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
                warnKrita << "WARNING: tryParseDescriptor: The points of the curve object contain \'Cnty\' flag which is unsupported by Krita";
                warnKrita << "        " << ppVar(type) << ppVar(key) << ppVar(path);

                child = child.nextSibling();
                continue;
            }

            if (type != "Double") {
                warnKrita << "Unknown point component type:" << ppVar(type) << ppVar(key) << ppVar(path);
                return false;
            }

            double value = KisDomUtils::toDouble(childEl.attribute("value", "0"));

            if (key == "Hrzn") {
                point.setX(value);
            } else if (key == "Vrtc") {
                point.setY(value);
            } else {
                warnKrita << "Unknown point key value:" << ppVar(key) << ppVar(path);
                return false;
            }

            child = child.nextSibling();
        }

        catcher.addPoint(path, point);

    } else if (classId == "Pnt ") {
        QPointF point;

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            QDomElement childEl = child.toElement();

            QString type = childEl.attribute("type", "<unknown>");
            QString key = childEl.attribute("key", "");
            QString unit = childEl.attribute("unit", "");

            if (type != "Double" && !(type == "UnitFloat" && unit == "#Prc")) {
                warnKrita << "Unknown point component type:" << ppVar(unit) << ppVar(type) << ppVar(key) << ppVar(path);
                return false;
            }

            double value = KisDomUtils::toDouble(childEl.attribute("value", "0"));

            if (key == "Hrzn") {
                point.setX(value);
            } else if (key == "Vrtc") {
                point.setY(value);
            } else {
                warnKrita << "Unknown point key value:" << ppVar(key) << ppVar(path);
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
                patternUuid = childEl.attribute("value", "").trimmed();
            }

            if (type == "KisPatternData" && key == "Data") {
                QDomNode dataNode = child.firstChild();

                if (!dataNode.isCDATASection()) {
                    warnKrita << "WARNING: failed to parse KisPatternData XML section!";
                    continue;
                }

                QDomCDATASection dataSection = dataNode.toCDATASection();
                QByteArray data = dataSection.data().toLatin1();
                data = QByteArray::fromBase64(data);
                data = qUncompress(data);

                if (data.isEmpty()) {
                    warnKrita << "WARNING: failed to parse KisPatternData XML section!";
                    continue;
                }

                patternData = data;
            }

            child = child.nextSibling();
        }

        if (!patternUuid.isEmpty() && !patternData.isEmpty()) {
            QString fileName = QString("%1.pat").arg(patternUuid);

            QSharedPointer<KoPattern> pattern(new KoPattern(fileName));

            QBuffer buffer(&patternData);
            buffer.open(QIODevice::ReadOnly);

            if (pattern->loadPatFromDevice(&buffer) && pattern->valid()) {
                catcher.addPattern(path, pattern, patternUuid);
            }
            else {
                warnKrita << "WARNING: failed to create pattern:" << ppVar(patternUuid) << ppVar(pattern);
            }
        } else {
            warnKrita << "WARNING: failed to load KisPattern XML section!" << ppVar(patternUuid);
        }

    } else if (classId == "Ptrn") { // reference to an existing pattern
        QString patternUuid;
        QString patternName;

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            QDomElement childEl = child.toElement();

            QString type = childEl.attribute("type", "<unknown>");
            QString key = childEl.attribute("key", "");

            if (type == "Text" && key == "Idnt") {
                patternUuid = childEl.attribute("value", "");
            } else if (type == "Text" && key == "Nm  ") {
                patternName = childEl.attribute("value", "");
            } else {
                warnKrita << "WARNING: unrecognized pattern-ref section key:" << ppVar(type) << ppVar(key);
            }

            child = child.nextSibling();
        }

        catcher.addPatternRef(path, patternUuid, patternName);

    } else if (classId == "Grdn") {
        QString gradientName;
        qreal gradientSmoothness = 100.0;

        QVector<qreal> startLocations;
        QVector<qreal> middleOffsets;
        QVector<KoColor> colors;
        QVector<KoGradientSegmentEndpointType> types;

        QVector<qreal> transpStartLocations;
        QVector<qreal> transpMiddleOffsets;
        QVector<qreal> transparencies;

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            QDomElement childEl = child.toElement();

            QString type = childEl.attribute("type", "<unknown>");
            QString key = childEl.attribute("key", "");

            if (type == "Text" && key == "Nm  ") {
                gradientName = childEl.attribute("value", "");
            } else if (type == "Enum" && key == "GrdF") {
                QString typeId = childEl.attribute("typeId", "");
                QString value = childEl.attribute("value", "");

                if (typeId != "GrdF" || value != "CstS") {
                    warnKrita << "WARNING: Unsupported gradient type (porbably, noise-based):" << value;
                    return true;
                }
            } else if (type == "Double" && key == "Intr") {
                double value = KisDomUtils::toDouble(childEl.attribute("value", "4096"));
                gradientSmoothness = 100.0 * value / 4096.0;
            } else if (type == "List" && key == "Clrs") {
                parseColorStopsList(childEl, startLocations, middleOffsets, colors, types);
            } else if (type == "List" && key == "Trns") {
                parseTransparencyStopsList(childEl, transpStartLocations, transpMiddleOffsets, transparencies);
            }

            child = child.nextSibling();
        }

        if (colors.size() < transparencies.size()) {
            const KoColor lastColor = !colors.isEmpty() ? colors.last() : KoColor();
            const KoGradientSegmentEndpointType lastType = !types.isEmpty() ? types.last() : COLOR_ENDPOINT;
            while (colors.size() != transparencies.size()) {
                const int index = colors.size();
                colors.append(lastColor);
                startLocations.append(transpStartLocations[index]);
                middleOffsets.append(transpMiddleOffsets[index]);
                types.append(lastType);
            }
        }

        if (colors.size() > transparencies.size()) {
            const qreal lastTransparency = !transparencies.isEmpty() ? transparencies.last() : 1.0;
            while (colors.size() != transparencies.size()) {
                const int index = transparencies.size();
                transparencies.append(lastTransparency);
                transpStartLocations.append(startLocations[index]);
                transpMiddleOffsets.append(middleOffsets[index]);
            }
        }

        if (colors.size() == 1) {
            colors.append(colors.last());
            startLocations.append(1.0);
            middleOffsets.append(0.5);
            types.append(COLOR_ENDPOINT);

            transparencies.append(transparencies.last());
            transpStartLocations.append(1.0);
            transpMiddleOffsets.append(0.5);
        }

        /**
         * Filenames in Krita cannot have slashes inside, but some of the
         * styles saved in 4.x days could have that. Here we just forcefully
         * crop the directory part of the gradient to make sure that it fits
         * the new policy.
         *
         * Since ASL doesn't use this name as any linkage (actually, gradients
         * are always embedded into the style) so we don't really care about
         * the contents of the filename field. It should just be somewhat unique.
         */
        const QString fileName = QFileInfo(gradientName).fileName() + ".ggr";
        QSharedPointer<KoSegmentGradient> gradient(new KoSegmentGradient(fileName));
        Q_UNUSED(gradientSmoothness);
        gradient->setName(gradientName);

        if (colors.size() >= 2) {
            for (int i = 1; i < colors.size(); i++) {
                KoColor startColor = colors[i - 1];
                KoColor endColor = colors[i];
                startColor.setOpacity(transparencies[i - 1]);
                endColor.setOpacity(transparencies[i]);

                qreal start = startLocations[i - 1];
                qreal end = startLocations[i];
                qreal middle = start + middleOffsets[i - 1] * (end - start);

                KoGradientSegmentEndpointType startType = types[i - 1];
                KoGradientSegmentEndpointType endType = types[i];

                gradient->createSegment(INTERP_LINEAR, COLOR_INTERP_RGB, start, end, middle, startColor, endColor, startType, endType);
            }
            gradient->setValid(true);
            gradient->updatePreview();
        } else {
            gradient->setValid(false);
        }

        catcher.addGradient(path, gradient);
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
        double v = KisDomUtils::toDouble(el.attribute("value", "0"));
        catcher.addDouble(buildPath(parentPath, key), v);
    } else if (type == "UnitFloat") {
        QString unit = el.attribute("unit", "<unknown>");
        double v = KisDomUtils::toDouble(el.attribute("value", "0"));
        catcher.addUnitFloat(buildPath(parentPath, key), unit, v);
    } else if (type == "Text") {
        QString v = el.attribute("value", "");
        catcher.addText(buildPath(parentPath, key), v);
    } else if (type == "Enum") {
        QString v = el.attribute("value", "");
        QString typeId = el.attribute("typeId", "<unknown>");
        catcher.addEnum(buildPath(parentPath, key), typeId, v);
    } else if (type == "Integer") {
        int v = KisDomUtils::toInt(el.attribute("value", "0"));
        catcher.addInteger(buildPath(parentPath, key), v);
    } else if (type == "Boolean") {
        int v = KisDomUtils::toInt(el.attribute("value", "0"));
        catcher.addBoolean(buildPath(parentPath, key), v);
    } else {
        warnKrita << "WARNING: XML (ASL) Unknown element type:" << type << ppVar(parentPath) << ppVar(key);
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
