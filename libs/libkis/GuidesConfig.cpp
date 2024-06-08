/*
 *  SPDX-FileCopyrightText: 2024 Grum999 <grum999@grum.fr>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "GuidesConfig.h"

#include <QDomDocument>
#include <QDomElement>

#include "kis_guides_config.h"

struct GuidesConfig::Private {
    Private() {}
    KisGuidesConfig *guidesConfig;
};

GuidesConfig::GuidesConfig(KisGuidesConfig *guidesConfig)
    : QObject(0)
    , d(new Private)
{
    d->guidesConfig = guidesConfig;
}

GuidesConfig::GuidesConfig(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->guidesConfig = new KisGuidesConfig();
}

GuidesConfig::~GuidesConfig()
{
    delete d;
}

bool GuidesConfig::operator==(const GuidesConfig &other) const
{
    return (d->guidesConfig == other.d->guidesConfig);
}

bool GuidesConfig::operator!=(const GuidesConfig &other) const
{
    return !(operator==(other));
}

QColor GuidesConfig::color() const
{
    return d->guidesConfig->guidesColor();
}

void GuidesConfig::setColor(const QColor &color) const
{
    d->guidesConfig->setGuidesColor(color);
}

QString GuidesConfig::lineType() const
{
    KisGuidesConfig::LineTypeInternal lineType = d->guidesConfig->guidesLineType();
    if(lineType == KisGuidesConfig::LineTypeInternal::LINE_DASHED) {
        return "dashed";
    }
    else if(lineType == KisGuidesConfig::LineTypeInternal::LINE_DOTTED) {
        return "dotted";
    }
    return "solid";
}

void GuidesConfig::setLineType(const QString &lineType)
{
    if (!d->guidesConfig) return;

    if(lineType == "dashed") {
        d->guidesConfig->setGuidesLineType(KisGuidesConfig::LineTypeInternal::LINE_DASHED);
    }
    else if(lineType == "dotted") {
        d->guidesConfig->setGuidesLineType(KisGuidesConfig::LineTypeInternal::LINE_DOTTED);
    }
    else {
        d->guidesConfig->setGuidesLineType(KisGuidesConfig::LineTypeInternal::LINE_SOLID);
    }
}

bool GuidesConfig::hasGuides() const
{
    return d->guidesConfig->hasGuides();
}

bool GuidesConfig::hasSamePositionAs(const GuidesConfig &guideConfig) const
{
    return d->guidesConfig->hasSamePositionAs(guideConfig.guidesConfig());
}

bool GuidesConfig::visible() const
{
    return d->guidesConfig->showGuides();
}

void GuidesConfig::setVisible(const bool value)
{
    d->guidesConfig->setShowGuides(value);
}

bool GuidesConfig::fromXml(const QString &xmlContent) const
{
    QDomDocument doc = QDomDocument();
    QString errorMsg {""};
    int errorLine {0}, errorColumn {0};

    if(doc.setContent(xmlContent, &errorMsg, &errorLine, &errorColumn)) {
        return d->guidesConfig->loadFromXml(doc.documentElement());
    }

    return false;
}

QString GuidesConfig::toXml() const
{
    QDomDocument doc = QDomDocument();
    QDomElement elt = d->guidesConfig->saveToXml(doc, "guides");
    doc.appendChild(elt);
    return doc.toString(2);
}

void GuidesConfig::removeAllGuides()
{
    d->guidesConfig->removeAllGuides();
}

bool GuidesConfig::locked() const
{
    return d->guidesConfig->lockGuides();
}

void GuidesConfig::setLocked(const bool locked)
{
    d->guidesConfig->setLockGuides(locked);
}

bool GuidesConfig::snap() const
{
    return d->guidesConfig->snapToGuides();
}

void GuidesConfig::setSnap(bool snap)
{
    d->guidesConfig->setSnapToGuides(snap);
}

void GuidesConfig::setHorizontalGuides(const QList<qreal> &lines)
{
    d->guidesConfig->setHorizontalGuideLines(lines);
}

void GuidesConfig::setVerticalGuides(const QList<qreal> &lines)
{
    d->guidesConfig->setVerticalGuideLines(lines);
}

QList<qreal> GuidesConfig::horizontalGuides() const
{
    return d->guidesConfig->horizontalGuideLines();
}

QList<qreal> GuidesConfig::verticalGuides() const
{
    return d->guidesConfig->verticalGuideLines();
}

KisGuidesConfig GuidesConfig::guidesConfig() const
{
    return *d->guidesConfig;
}
