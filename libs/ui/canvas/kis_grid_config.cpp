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

#include "kis_grid_config.h"

#include <QDomElement>

#include "kis_config.h"
#include "kis_dom_utils.h"


struct KisGridConfigStaticRegistrar {
    KisGridConfigStaticRegistrar() {
        qRegisterMetaType<KisGridConfig>("KisGridConfig");
    }
};
static KisGridConfigStaticRegistrar __registrar;

Q_GLOBAL_STATIC(KisGridConfig, staticDefaultObject)

const KisGridConfig& KisGridConfig::defaultGrid()
{
    staticDefaultObject->loadStaticData();
    return *staticDefaultObject;
}

void KisGridConfig::loadStaticData()
{
    KisConfig cfg(true);

    m_lineTypeMain = LineTypeInternal(cfg.getGridMainStyle());
    m_lineTypeSubdivision = LineTypeInternal(cfg.getGridSubdivisionStyle());

    m_colorMain = cfg.getGridMainColor();
    m_colorSubdivision = cfg.getGridSubdivisionColor();
}

void KisGridConfig::saveStaticData() const
{
    KisConfig cfg(false);
    cfg.setGridMainStyle(m_lineTypeMain);
    cfg.setGridSubdivisionStyle(m_lineTypeSubdivision);
    cfg.setGridMainColor(m_colorMain);
    cfg.setGridSubdivisionColor(m_colorSubdivision);
}

QDomElement KisGridConfig::saveDynamicDataToXml(QDomDocument& doc, const QString &tag) const
{
    QDomElement gridElement = doc.createElement(tag);
    KisDomUtils::saveValue(&gridElement, "showGrid", m_showGrid);
    KisDomUtils::saveValue(&gridElement, "snapToGrid", m_snapToGrid);
    KisDomUtils::saveValue(&gridElement, "offset", m_offset);
    KisDomUtils::saveValue(&gridElement, "spacing", m_spacing);
    KisDomUtils::saveValue(&gridElement, "offsetAspectLocked", m_offsetAspectLocked);
    KisDomUtils::saveValue(&gridElement, "spacingAspectLocked", m_spacingAspectLocked);
    KisDomUtils::saveValue(&gridElement, "subdivision", m_subdivision);
    KisDomUtils::saveValue(&gridElement, "angleLeft", m_angleLeft);
    KisDomUtils::saveValue(&gridElement, "angleRight", m_angleRight);
    KisDomUtils::saveValue(&gridElement, "cellSpacing", m_cellSpacing);
    KisDomUtils::saveValue(&gridElement, "gridType", m_gridType);


    return gridElement;
}

bool KisGridConfig::loadDynamicDataFromXml(const QDomElement &gridElement)
{
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
    result &= KisDomUtils::loadValue(gridElement, "gridType", &m_gridType);

    return result;
}
