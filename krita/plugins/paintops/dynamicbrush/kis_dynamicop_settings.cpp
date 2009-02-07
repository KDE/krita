
/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_dynamicop_settings.h"

#include <QWidget>

#include <kis_bookmarked_configuration_manager.h>
#include <kis_bookmarked_configurations_model.h>

#include "kis_dynamicop_settings_widget.h"
#include "ui_DynamicBrushOptions.h"


KisDynamicOpSettings::KisDynamicOpSettings(KisDynamicOpSettingsWidget* widget, KisBookmarkedConfigurationManager* shapeBookmarksManager, KisBookmarkedConfigurationManager* coloringBookmarksManager)
    : KisPaintOpSettings( widget )
{
    Q_ASSERT( widget );
    m_optionsWidget = widget;
    m_optionsWidget->writeConfiguration( this );
    m_shapeBookmarksModel = new KisBookmarkedConfigurationsModel(shapeBookmarksManager);
    m_coloringBookmarksModel = new KisBookmarkedConfigurationsModel(coloringBookmarksManager);

    m_optionsWidget->m_uiOptions->comboBoxShapePrograms->setModel(m_shapeBookmarksModel);
    m_optionsWidget->m_uiOptions->comboBoxColoringPrograms->setModel(m_coloringBookmarksModel);


}

KisDynamicOpSettings::~KisDynamicOpSettings()
{
}

bool KisDynamicOpSettings::paintIncremental()
{
    return true;
}



// TEMP
KisDynamicBrush* KisDynamicOpSettings::createBrush(KisPainter *painter) const
{
#if 0
    KisDynamicBrush* current = new KisDynamicBrush(i18n("example"));
    // Init shape program
    QModelIndex shapeModelIndex = m_shapeBookmarksModel->index(
                                      m_uiOptions->comboBoxShapePrograms->currentIndex(), 0);
    KisDynamicShapeProgram* shapeProgram = static_cast<KisDynamicShapeProgram*>(m_shapeBookmarksModel->configuration(shapeModelIndex));
    Q_ASSERT(shapeProgram);
    current->setShapeProgram(shapeProgram);
    // Init coloring program
    QModelIndex coloringModelIndex = m_coloringBookmarksModel->index(
                                         m_uiOptions->comboBoxColoringPrograms->currentIndex(), 0);
    KisDynamicColoringProgram* coloringProgram = static_cast<KisDynamicColoringProgram*>(m_coloringBookmarksModel->configuration(coloringModelIndex));
    Q_ASSERT(coloringProgram);
    current->setColoringProgram(coloringProgram);
    // Init shape
    switch (m_uiOptions->comboBoxShapes->currentIndex()) {
    case 1:
        current->setShape(new KisBristleShape());
        break;
    default:
    case 0: {
#if 0 // XXX: Fix!
        current->setShape(new KisDabShape(painter->brush()));
#endif
    }
    }
    // Init coloring
    switch (m_uiOptions->comboBoxColoring->currentIndex()) {
    case 3:
        current->setColoring(new KisTotalRandomColoring());
        break;
    case 2:
        current->setColoring(new KisUniformRandomColoring());
        break;
    case 1:
        current->setColoring(new KisGradientColoring(painter->gradient() , painter->paintColor().colorSpace()));
        break;
    default:
    case 0: {
        current->setColoring(new KisPlainColoring(painter->backgroundColor() , painter->paintColor()));
    }
    }

    return current;
#endif
}

void KisDynamicOpSettings::fromXML(const QDomElement& elt)
{
    // First, call the parent class fromXML to make sure all the
    // properties are saved to the map
    KisPaintOpSettings::fromXML( elt );

    // Then load the properties for all widgets
    m_optionsWidget->setConfiguration( this );

#if 0
    m_uiOptions->comboBoxShapes->setCurrentIndex(elt.attribute("shapeType", "0").toInt());
    m_uiOptions->comboBoxColoring->setCurrentIndex(elt.attribute("coloringType", "0").toInt());
    // XXX: load the shape program / coloring program
#endif
}

void KisDynamicOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    // First, make sure all the option widgets have saved their state
    // to the property configuration
    KisPropertiesConfiguration * settings = m_optionsWidget->configuration();

    // Then call the parent class fromXML
    settings->KisPropertiesConfiguration::toXML( doc, rootElt );

    delete settings;

#if 0
    rootElt.setAttribute("shapeType", m_uiOptions->comboBoxShapes->currentIndex());
    rootElt.setAttribute("coloringType", m_uiOptions->comboBoxColoring->currentIndex());
    // Save shape program
    QDomElement shapeElt = doc.createElement("Shape");
    rootElt.appendChild(shapeElt);

    QModelIndex shapeModelIndex = m_shapeBookmarksModel->index(
                                      m_uiOptions->comboBoxShapePrograms->currentIndex(), 0);
    KisDynamicShapeProgram* shapeProgram = static_cast<KisDynamicShapeProgram*>(m_shapeBookmarksModel->configuration(shapeModelIndex));
    Q_ASSERT(shapeProgram);

    shapeProgram->toXML(doc, shapeElt);

    delete shapeProgram;

    // Save Coloring program
    QDomElement coloringElt = doc.createElement("Coloring");
    rootElt.appendChild(coloringElt);

    QModelIndex coloringModelIndex = m_coloringBookmarksModel->index(
                                         m_uiOptions->comboBoxColoringPrograms->currentIndex(), 0);
    KisDynamicColoringProgram* coloringProgram = static_cast<KisDynamicColoringProgram*>(m_coloringBookmarksModel->configuration(coloringModelIndex));
    Q_ASSERT(coloringProgram);

    coloringProgram->toXML(doc, coloringElt);

    delete coloringProgram;
#endif
}

KisPaintOpSettingsSP KisDynamicOpSettings::clone() const
{
    KisPaintOpSettingsSP settings = dynamic_cast<KisPaintOpSettings*>( m_optionsWidget->configuration() );
    return settings;
#if 0
    return new KisDynamicOpSettings(0, m_shapeBookmarksModel->bookmarkedConfigurationManager(), m_coloringBookmarksModel->bookmarkedConfigurationManager());
#endif
}
