/*
 *  SPDX-FileCopyrightText: 2024 Grum999 <grum999@grum.fr>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "GridConfig.h"

#include <QDomDocument>
#include <QDomElement>

#include "kis_grid_config.h"
#include "kis_debug.h"

struct GridConfig::Private {
    Private() {}
    KisGridConfig *gridConfig;
};

GridConfig::GridConfig(KisGridConfig *gridConfig)
    : QObject(0)
    , d(new Private)
{
    d->gridConfig = gridConfig;
}

GridConfig::GridConfig(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->gridConfig = new KisGridConfig();
}

GridConfig::~GridConfig()
{
    delete d;
}

bool GridConfig::operator==(const GridConfig &other) const
{
    return (d->gridConfig == other.d->gridConfig);
}

bool GridConfig::operator!=(const GridConfig &other) const
{
    return !(operator==(other));
}


bool GridConfig::visible() const
{
    return d->gridConfig->showGrid();
}

void GridConfig::setVisible(bool visible)
{
    d->gridConfig->setShowGrid(visible);
}

bool GridConfig::snap() const
{
    return d->gridConfig->showGrid();
}

void GridConfig::setSnap(bool snap)
{
    d->gridConfig->setSnapToGrid(snap);
}

QPoint GridConfig::offset() const
{
    return d->gridConfig->offset();
}

void GridConfig::setOffset(QPoint offset)
{
    QPoint rangeOffset = QPoint(offset);
    rangeOffset.setX(qBound(0, rangeOffset.x(), 500));
    rangeOffset.setY(qBound(0, rangeOffset.y(), 500));
    d->gridConfig->setOffset(rangeOffset);
}

QPoint GridConfig::spacing() const
{
    return d->gridConfig->spacing();
}

void GridConfig::setSpacing(QPoint spacing)
{
    QPoint rangeSpacing = QPoint(spacing);
    if (rangeSpacing.x() < 1) {
        rangeSpacing.setX(1);
    }
    if (rangeSpacing.y() < 1) {
        rangeSpacing.setY(1);
    }
    d->gridConfig->setSpacing(rangeSpacing);
}

bool GridConfig::spacingActiveHorizontal() const
{
    return d->gridConfig->xSpacingActive();
}

void GridConfig::setSpacingActiveHorizontal(bool active)
{
    d->gridConfig->setXSpacingActive(active);
}

bool GridConfig::spacingActiveVertical() const
{
    return d->gridConfig->ySpacingActive();
}

void GridConfig::setSpacingActiveVertical(bool active)
{
    d->gridConfig->setYSpacingActive(active);
}

int GridConfig::subdivision() const
{
    return d->gridConfig->subdivision();
}

void GridConfig::setSubdivision(int subdivision)
{
    d->gridConfig->setSubdivision(qBound(1, subdivision, 10));
}

qreal GridConfig::angleLeft() const
{
    return d->gridConfig->angleLeft();
}

void GridConfig::setAngleLeft(qreal angleLeft)
{
    d->gridConfig->setAngleLeft(qBound(0.0, angleLeft, 89.0));
}

qreal GridConfig::angleRight() const
{
    return d->gridConfig->angleRight();
}

void GridConfig::setAngleRight(qreal angleRight)
{
    d->gridConfig->setAngleRight(qBound(0.0, angleRight, 89.0));
}

bool GridConfig::angleLeftActive() const
{
    return d->gridConfig->angleLeftActive();
}

void GridConfig::setAngleLeftActive(bool active)
{
    d->gridConfig->setAngleLeftActive(active);
}

bool GridConfig::angleRightActive() const
{
    return d->gridConfig->angleRightActive();
}

void GridConfig::setAngleRightActive(bool active)
{
    d->gridConfig->setAngleRightActive(active);
}

int GridConfig::cellSpacing() const
{
    return d->gridConfig->cellSpacing();
}

void GridConfig::setCellSpacing(int cellSpacing)
{
    d->gridConfig->setCellSpacing(qBound(10, cellSpacing, 1000));
}

int GridConfig::cellSize() const
{
    return d->gridConfig->cellSize();
}

void GridConfig::setCellSize(int cellSize)
{
    d->gridConfig->setCellSize(qBound(10, cellSize, 1000));
}

QString GridConfig::type() const
{
    KisGridConfig::GridType type = d->gridConfig->gridType();
    if(type == KisGridConfig::GridType::GRID_ISOMETRIC) {
        return "isometric";
    }
    else if(type == KisGridConfig::GridType::GRID_ISOMETRIC_LEGACY) {
        return "isometric_legacy";
    }
    return "rectangular";
}

void GridConfig::setType(const QString &gridType)
{
    if(gridType == "isometric") {
        d->gridConfig->setGridType(KisGridConfig::GridType::GRID_ISOMETRIC);
    }
    else if(gridType == "isometric_legacy") {
        d->gridConfig->setGridType(KisGridConfig::GridType::GRID_ISOMETRIC_LEGACY);
    }
    else {
        d->gridConfig->setGridType(KisGridConfig::GridType::GRID_RECTANGULAR);
    }
}

bool GridConfig::offsetAspectLocked() const
{
    return d->gridConfig->offsetAspectLocked();
}

void GridConfig::setOffsetAspectLocked(bool offsetAspectLocked)
{
    d->gridConfig->setOffsetAspectLocked(offsetAspectLocked);
}

bool GridConfig::spacingAspectLocked() const
{
    return d->gridConfig->spacingAspectLocked();
}

void GridConfig::setSpacingAspectLocked(bool spacingAspectLocked)
{
    d->gridConfig->setSpacingAspectLocked(spacingAspectLocked);
}

bool GridConfig::angleAspectLocked() const
{
    return d->gridConfig->angleAspectLocked();
}

void GridConfig::setAngleAspectLocked(bool angleAspectLocked)
{
    d->gridConfig->setAngleAspectLocked(angleAspectLocked);
}

QString GridConfig::lineTypeMain() const
{
    KisGridConfig::LineTypeInternal type = d->gridConfig->lineTypeMain();
    if(type == KisGridConfig::LineTypeInternal::LINE_DASHED) {
        return "dashed";
    }
    else if(type == KisGridConfig::LineTypeInternal::LINE_DOTTED) {
        return "dotted";
    }
    return "solid";
}

void GridConfig::setLineTypeMain(const QString &lineType)
{
    if(lineType == "dashed") {
        d->gridConfig->setLineTypeMain(KisGridConfig::LineTypeInternal::LINE_DASHED);
    }
    else if(lineType == "dotted") {
        d->gridConfig->setLineTypeMain(KisGridConfig::LineTypeInternal::LINE_DOTTED);
    }
    else {
        d->gridConfig->setLineTypeMain(KisGridConfig::LineTypeInternal::LINE_SOLID);
    }
}

QString GridConfig::lineTypeSubdivision() const
{
    KisGridConfig::LineTypeInternal type = d->gridConfig->lineTypeSubdivision();
    if(type == KisGridConfig::LineTypeInternal::LINE_DASHED) {
        return "dashed";
    }
    else if(type == KisGridConfig::LineTypeInternal::LINE_DOTTED) {
        return "dotted";
    }
    return "solid";
}

void GridConfig::setLineTypeSubdivision(const QString &lineType)
{
    if(lineType == "dashed") {
        d->gridConfig->setLineTypeSubdivision(KisGridConfig::LineTypeInternal::LINE_DASHED);
    }
    else if(lineType == "dotted") {
        d->gridConfig->setLineTypeSubdivision(KisGridConfig::LineTypeInternal::LINE_DOTTED);
    }
    else {
        d->gridConfig->setLineTypeSubdivision(KisGridConfig::LineTypeInternal::LINE_SOLID);
    }
}

QString GridConfig::lineTypeVertical() const
{
    KisGridConfig::LineTypeInternal type = d->gridConfig->lineTypeVertical();
    if(type == KisGridConfig::LineTypeInternal::LINE_DASHED) {
        return "dashed";
    }
    else if(type == KisGridConfig::LineTypeInternal::LINE_DOTTED) {
        return "dotted";
    }
    else if(type == KisGridConfig::LineTypeInternal::LINE_NONE) {
        return "none";
    }
    return "solid";
}

void GridConfig::setLineTypeVertical(const QString &lineType)
{
    if(lineType == "dashed") {
        d->gridConfig->setLineTypeVertical(KisGridConfig::LineTypeInternal::LINE_DASHED);
    }
    else if(lineType == "dotted") {
        d->gridConfig->setLineTypeVertical(KisGridConfig::LineTypeInternal::LINE_DOTTED);
    }
    else if(lineType == "none") {
        d->gridConfig->setLineTypeVertical(KisGridConfig::LineTypeInternal::LINE_NONE);
    }
    else {
        d->gridConfig->setLineTypeVertical(KisGridConfig::LineTypeInternal::LINE_SOLID);
    }
}


QColor GridConfig::colorMain() const
{
    return d->gridConfig->colorMain();
}

void GridConfig::setColorMain(QColor colorMain)
{
    d->gridConfig->setColorMain(colorMain);
}

QColor GridConfig::colorSubdivision() const
{
    return d->gridConfig->colorSubdivision();
}

void GridConfig::setColorSubdivision(QColor colorSubdivision)
{
    d->gridConfig->setColorSubdivision(colorSubdivision);
}

QColor GridConfig::colorVertical() const
{
    return d->gridConfig->colorVertical();
}

void GridConfig::setColorVertical(QColor colorVertical)
{
    d->gridConfig->setColorVertical(colorVertical);
}

bool GridConfig::fromXml(const QString &xmlContent) const
{
    QDomDocument doc = QDomDocument();
    QString errorMsg {""};
    int errorLine {0}, errorColumn {0};

    if(doc.setContent(xmlContent, &errorMsg, &errorLine, &errorColumn)) {

        return d->gridConfig->loadDynamicDataFromXml(doc.documentElement());
    }

    return false;
}

QString GridConfig::toXml() const
{
    QDomDocument doc = QDomDocument();
    QDomElement elt = d->gridConfig->saveDynamicDataToXml(doc, "grid");
    doc.appendChild(elt);
    return doc.toString(2);
}


KisGridConfig GridConfig::gridConfig() const
{
    return *d->gridConfig;
}


