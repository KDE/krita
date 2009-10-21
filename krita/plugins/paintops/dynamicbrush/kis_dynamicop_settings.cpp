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
#include <QDomElement>

#include <kis_bookmarked_configuration_manager.h>
#include <kis_bookmarked_configurations_model.h>
#include <kis_painter.h>

#include "kis_dynamic_coloring.h"
#include "kis_dynamic_brush.h"
#include "kis_dynamicop_settings_widget.h"
#include "ui_DynamicBrushOptions.h"
#include "kis_dynamic_shape_program.h"
#include "kis_dynamic_coloring_program.h"
#include "kis_dab_shape.h"
#include "kis_brush.h"
#include "kis_bristle_shape.h"

KisDynamicOpSettings::KisDynamicOpSettings(KisDynamicOpSettingsWidget* widget, KisBookmarkedConfigurationsModel* shapeBookmarksManager, KisBookmarkedConfigurationsModel* coloringBookmarksManager)
        : KisPaintOpSettings(widget)
{
    Q_ASSERT(widget);
    m_options = widget;
    m_options->writeConfiguration(this);
    m_shapeBookmarksModel = shapeBookmarksManager;
    m_coloringBookmarksModel = coloringBookmarksManager;

    int s = m_options->m_uiOptions->comboBoxShapePrograms->currentIndex();
    if (s < 0) s = 0;
    m_options->m_uiOptions->comboBoxShapePrograms->setModel(m_shapeBookmarksModel);
    m_options->m_uiOptions->comboBoxShapePrograms->setCurrentIndex(s);
    int c = m_options->m_uiOptions->comboBoxColoringPrograms->currentIndex();
    if (c < 0) c = 0;
    m_options->m_uiOptions->comboBoxColoringPrograms->setModel(m_coloringBookmarksModel);
    m_options->m_uiOptions->comboBoxColoringPrograms->setCurrentIndex(c);
}

KisDynamicOpSettings::~KisDynamicOpSettings()
{
}

bool KisDynamicOpSettings::paintIncremental()
{
    return true;
}

KisDynamicBrush* KisDynamicOpSettings::createBrush(KisPainter *painter) const
{
    KisDynamicBrush* current = new KisDynamicBrush(i18n("example"));
    // Init shape program
    QModelIndex shapeModelIndex = m_shapeBookmarksModel->index(
                                      m_options->m_uiOptions->comboBoxShapePrograms->currentIndex(), 0);
    KisDynamicShapeProgram* shapeProgram =
        static_cast<KisDynamicShapeProgram*>(m_shapeBookmarksModel->configuration(shapeModelIndex));
    Q_ASSERT(shapeProgram);
    current->setShapeProgram(shapeProgram);
    // Init coloring program
    dbgKrita << "Load color programs at: " << m_options->m_uiOptions->comboBoxColoringPrograms->currentIndex();
    QModelIndex coloringModelIndex = m_coloringBookmarksModel->index(
                                         m_options->m_uiOptions->comboBoxColoringPrograms->currentIndex(), 0);
    KisDynamicColoringProgram* coloringProgram = static_cast<KisDynamicColoringProgram*>(m_coloringBookmarksModel->configuration(coloringModelIndex));
    Q_ASSERT(coloringProgram);
    current->setColoringProgram(coloringProgram);


    // Init shape
    switch (m_options->m_uiOptions->comboBoxShapes->currentIndex()) {
    default:
    case 0: {
        current->setShape(new KisDabShape(m_options->m_uiOptions->brushOptions->brush()));
        break;
    }
    case 1:
        // fall-through
        current->setShape(new KisBristleShape());
        break;
    }
    // Init coloring
    switch (m_options->m_uiOptions->comboBoxColoring->currentIndex()) {
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
}

void KisDynamicOpSettings::fromXML(const QDomElement& elt)
{

    m_options->m_uiOptions->comboBoxShapes->setCurrentIndex(elt.attribute("shapeType", "0").toInt());
    m_options->m_uiOptions->comboBoxColoring->setCurrentIndex(elt.attribute("coloringType", "0").toInt());
#if 0
    // The following is not good, nothing prove that the programs contained in elt are the same as in the registry
    QDomNode nShape = elt.firstChildElement("Shape");
    if (nShape.isElement()) {
        QDomElement eShape = nShape.toElement();
        QString name = eShape.attribute("name");
        m_options->m_uiOptions->comboBoxShapePrograms->setCurrentIndex(
            m_shapeBookmarksModel->indexFor(name).row());
        dbgPlugins << name << " " << m_shapeBookmarksModel->indexFor(name).row();
    }
    abort();
    QDomNode nColoring = elt.firstChildElement("Coloring");
    if (nColoring.isElement()) {
        QDomElement eColoring = nColoring.toElement();
        QString name = eColoring.attribute("name");
        m_options->m_uiOptions->comboBoxColoringPrograms->setCurrentIndex(
            m_coloringBookmarksModel->indexFor(name).row());
    }
#endif
    qFatal("Unimplemented");
}

void KisDynamicOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{

    rootElt.setAttribute("shapeType", m_options->m_uiOptions->comboBoxShapes->currentIndex());
    rootElt.setAttribute("coloringType", m_options->m_uiOptions->comboBoxColoring->currentIndex());
    // Save shape program
    QDomElement shapeElt = doc.createElement("Shape");
    rootElt.appendChild(shapeElt);

    dbgKrita << m_options->m_uiOptions->comboBoxShapePrograms->currentIndex();
    QModelIndex shapeModelIndex = m_shapeBookmarksModel->index(
                                      m_options->m_uiOptions->comboBoxShapePrograms->currentIndex(), 0);
    KisDynamicShapeProgram* shapeProgram = static_cast<KisDynamicShapeProgram*>(m_shapeBookmarksModel->configuration(shapeModelIndex));
    Q_ASSERT(shapeProgram);

    shapeProgram->toXML(doc, shapeElt);

    delete shapeProgram;

    // Save Coloring program
    QDomElement coloringElt = doc.createElement("Coloring");
    rootElt.appendChild(coloringElt);

    QModelIndex coloringModelIndex = m_coloringBookmarksModel->index(
                                         m_options->m_uiOptions->comboBoxColoringPrograms->currentIndex(), 0);
    KisDynamicColoringProgram* coloringProgram = static_cast<KisDynamicColoringProgram*>(m_coloringBookmarksModel->configuration(coloringModelIndex));
    Q_ASSERT(coloringProgram);

    coloringProgram->toXML(doc, coloringElt);

    delete coloringProgram;

}

KisPaintOpSettingsSP KisDynamicOpSettings::clone() const
{
    return new KisDynamicOpSettings(m_options,
                                    m_shapeBookmarksModel,
                                    m_coloringBookmarksModel);

}
