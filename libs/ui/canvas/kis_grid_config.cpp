/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_grid_config.h"

#include <QDomElement>
#include <QtMath>

#include "kis_config.h"
#include "kis_dom_utils.h"
#include "kis_algebra_2d.h"
#include <KisStaticInitializer.h>

KIS_DECLARE_STATIC_INITIALIZER {
    qRegisterMetaType<KisGridConfig>("KisGridConfig");
}

Q_GLOBAL_STATIC(KisGridConfig, staticDefaultObject)

const KisGridConfig& KisGridConfig::defaultGrid()
{
    staticDefaultObject->loadStaticData();
    return *staticDefaultObject;
}

void KisGridConfig::transform(const QTransform &transform)
{
    if (transform.type() >= QTransform::TxShear) return;

    KisAlgebra2D::DecomposedMatrix m(transform);

    if (m_gridType == GRID_RECTANGULAR) {
        QTransform t = m.scaleTransform();

        const qreal eps = 1e-3;
        const qreal wrappedRotation = KisAlgebra2D::wrapValue(m.angle, 90.0);
        if (wrappedRotation <= eps || wrappedRotation >= 90.0 - eps) {
            t *= m.rotateTransform();
        }

        m_spacing = KisAlgebra2D::abs(t.map(m_spacing));
        // Transform map may round spacing down to 0, but it must be at least 1
        m_spacing.setX(qMax(1, m_spacing.x()));
        m_spacing.setY(qMax(1, m_spacing.y()));

    } else if (m_gridType == GRID_ISOMETRIC_LEGACY) {
        if (qFuzzyCompare(m.scaleX, m.scaleY)) {
            m_cellSpacing = qRound(qAbs(m_cellSpacing * m.scaleX));
        }
    }
    m_offset = KisAlgebra2D::wrapValue(transform.map(m_offset), m_spacing);
}

void KisGridConfig::loadStaticData()
{
    KisConfig cfg(true);

    m_lineTypeMain = LineTypeInternal(cfg.getGridMainStyle());
    m_lineTypeSubdivision = LineTypeInternal(cfg.getGridSubdivisionStyle());
    m_lineTypeIsoVertical = LineTypeInternal(cfg.getGridIsoVerticalStyle());

    m_colorMain = cfg.getGridMainColor();
    m_colorSubdivision = cfg.getGridSubdivisionColor();
    m_colorIsoVertical = cfg.getGridIsoVerticalColor();

    m_spacing = cfg.getDefaultGridSpacing();
}

void KisGridConfig::saveStaticData() const
{
    KisConfig cfg(false);
    cfg.setGridMainStyle(m_lineTypeMain);
    cfg.setGridSubdivisionStyle(m_lineTypeSubdivision);
    cfg.setGridIsoVerticalStyle(m_lineTypeIsoVertical);
    cfg.setGridMainColor(m_colorMain);
    cfg.setGridSubdivisionColor(m_colorSubdivision);
    cfg.setGridIsoVerticalColor(m_colorIsoVertical);
}

QDomElement KisGridConfig::saveDynamicDataToXml(QDomDocument& doc, const QString &tag) const
{
    QDomElement gridElement = doc.createElement(tag);
    KisDomUtils::saveValue(&gridElement, "showGrid", m_showGrid);
    KisDomUtils::saveValue(&gridElement, "snapToGrid", m_snapToGrid);
    KisDomUtils::saveValue(&gridElement, "offsetActive", m_offsetActive);
    KisDomUtils::saveValue(&gridElement, "offset", m_offset);
    KisDomUtils::saveValue(&gridElement, "spacing", m_spacing);
    KisDomUtils::saveValue(&gridElement, "xSpacingActive", m_xSpacingActive);
    KisDomUtils::saveValue(&gridElement, "ySpacingActive", m_ySpacingActive);
    KisDomUtils::saveValue(&gridElement, "offsetAspectLocked", m_offsetAspectLocked);
    KisDomUtils::saveValue(&gridElement, "spacingAspectLocked", m_spacingAspectLocked);
    KisDomUtils::saveValue(&gridElement, "subdivision", m_subdivision);
    KisDomUtils::saveValue(&gridElement, "angleLeft", m_angleLeft);
    KisDomUtils::saveValue(&gridElement, "angleRight", m_angleRight);
    KisDomUtils::saveValue(&gridElement, "angleLeftActive", m_angleLeftActive);
    KisDomUtils::saveValue(&gridElement, "angleRightActive", m_angleRightActive);
    KisDomUtils::saveValue(&gridElement, "angleAspectLocked", m_angleAspectLocked);
    KisDomUtils::saveValue(&gridElement, "cellSpacing", m_cellSpacing);
    KisDomUtils::saveValue(&gridElement, "cellSize", m_cellSize);
    KisDomUtils::saveValue(&gridElement, "gridType", m_gridType);

    KisDomUtils::saveValue(&gridElement, "colorMain", m_colorMain);
    KisDomUtils::saveValue(&gridElement, "colorSubdivision", m_colorSubdivision);
    KisDomUtils::saveValue(&gridElement, "colorVertical", m_colorIsoVertical);
    KisDomUtils::saveValue(&gridElement, "lineTypeMain", m_lineTypeMain);
    KisDomUtils::saveValue(&gridElement, "lineTypeSubdivision", m_lineTypeSubdivision);
    KisDomUtils::saveValue(&gridElement, "lineTypeVertical", m_lineTypeIsoVertical);

    return gridElement;
}

bool KisGridConfig::loadDynamicDataFromXml(const QDomElement &gridElement)
{
    KisConfig cfg(true);
    bool result = true;

    result &= KisDomUtils::loadValue(gridElement, "showGrid", &m_showGrid);
    result &= KisDomUtils::loadValue(gridElement, "snapToGrid", &m_snapToGrid);
    result &= KisDomUtils::loadValue(gridElement, "offset", &m_offset);
    result &= KisDomUtils::loadValue(gridElement, "spacing", &m_spacing);
    result &= KisDomUtils::loadValue(gridElement, "offsetAspectLocked", &m_offsetAspectLocked);
    result &= KisDomUtils::loadValue(gridElement, "spacingAspectLocked", &m_spacingAspectLocked);
    result &= KisDomUtils::loadValue(gridElement, "subdivision", &m_subdivision);
    result &= KisDomUtils::loadValue(gridElement, "angleLeft", &m_angleLeft);
    result &= KisDomUtils::loadValue(gridElement, "angleRight", &m_angleRight);
    result &= KisDomUtils::loadValue(gridElement, "cellSpacing", &m_cellSpacing);
    result &= KisDomUtils::loadValue(gridElement, "gridType", (int*)(&m_gridType));

    // following variables may not be present in older files; do not update result variable
    KisDomUtils::loadValue(gridElement, "offsetActive", &m_offsetActive);
    KisDomUtils::loadValue(gridElement, "xSpacingActive", &m_xSpacingActive);
    KisDomUtils::loadValue(gridElement, "ySpacingActive", &m_ySpacingActive);
    KisDomUtils::loadValue(gridElement, "angleLeftActive", &m_angleLeftActive);
    KisDomUtils::loadValue(gridElement, "angleRightActive", &m_angleRightActive);
    KisDomUtils::loadValue(gridElement, "angleAspectLocked", &m_angleAspectLocked);
    KisDomUtils::loadValue(gridElement, "cellSize", &m_cellSize);

    int lineTypeMain = cfg.getGridMainStyle();
    KisDomUtils::loadValue(gridElement, "lineTypeMain", &lineTypeMain);
    m_lineTypeMain = LineTypeInternal(lineTypeMain);

    int lineTypeSubdivision = cfg.getGridSubdivisionStyle();
    KisDomUtils::loadValue(gridElement, "lineTypeSubdivision", &lineTypeSubdivision);
    m_lineTypeSubdivision = LineTypeInternal(lineTypeSubdivision);

    int lineTypeVertical = cfg.getGridIsoVerticalStyle();
    KisDomUtils::loadValue(gridElement, "lineTypeVertical", &lineTypeVertical);
    m_lineTypeIsoVertical = LineTypeInternal(lineTypeVertical);

    m_colorMain = cfg.getGridMainColor();
    KisDomUtils::loadValue(gridElement, "colorMain", &m_colorMain);

    m_colorSubdivision = cfg.getGridSubdivisionColor();
    KisDomUtils::loadValue(gridElement, "colorSubdivision", &m_colorSubdivision);

    m_colorIsoVertical = cfg.getGridIsoVerticalColor();
    KisDomUtils::loadValue(gridElement, "colorVertical", &m_colorIsoVertical);

    updatePenStyle(&m_penMain, m_colorMain, m_lineTypeMain);
    updatePenStyle(&m_penSubdivision, m_colorSubdivision, m_lineTypeSubdivision);
    updatePenStyle(&m_penVertical, m_colorIsoVertical, m_lineTypeIsoVertical);
    updateTrigoCache();

    return result;
}

void KisGridConfig::updatePenStyle(QPen *pen, QColor color, LineTypeInternal type)
{
    pen->setColor(color);

    if (type == LINE_DASHED) {
        QVector<qreal> dashes;
        dashes << 5 << 5;
        pen->setDashPattern(dashes);
    } else if (type == LINE_DOTTED) {
        pen->setStyle(Qt::DotLine);
    } else if (type == LINE_NONE) {
        pen->setStyle(Qt::NoPen);
    } else {
        // assume it's SOLID by default
        pen->setStyle(Qt::SolidLine);
    }
}

void KisGridConfig::updateTrigoCache()
{
    // Here some variable needed to render grid that can be calculated when grid settings in done, instead
    // of doing recalculation on every canvas refresh
    const qreal cosAngleRight = qCos(qDegreesToRadians(m_angleRight));
    const qreal cosAngleLeft = qCos(qDegreesToRadians(m_angleLeft));

    m_trigoCache.tanAngleRight = qTan(qDegreesToRadians(m_angleRight));
    m_trigoCache.correctedAngleRightCellSize = m_cellSize * (qSin(qDegreesToRadians(m_angleLeft)) + cosAngleLeft * m_trigoCache.tanAngleRight);
    if (m_angleRight > 0.0) {
        m_trigoCache.correctedAngleRightOffsetX = m_offset.x() * m_trigoCache.tanAngleRight;
    } else {
        m_trigoCache.correctedAngleRightOffsetX = m_offset.x();
    }

    m_trigoCache.tanAngleLeft = qTan(qDegreesToRadians(m_angleLeft));
    m_trigoCache.correctedAngleLeftCellSize = m_cellSize * (qSin(qDegreesToRadians(m_angleRight)) + cosAngleRight * m_trigoCache.tanAngleLeft);
    if (m_angleLeft > 0.0) {
        m_trigoCache.correctedAngleLeftOffsetX = m_offset.x() * m_trigoCache.tanAngleLeft;
    } else {
        m_trigoCache.correctedAngleLeftOffsetX = m_offset.x();
    }

    if (m_angleRight == m_angleLeft && m_lineTypeIsoVertical != LINE_NONE) {
        m_trigoCache.verticalSpace = m_subdivision * m_cellSize * (cosAngleLeft + cosAngleRight) / 2;
    } else {
        // allow vertical grid line only if angle left and right are the same
        m_trigoCache.verticalSpace = 0;
    }
}
