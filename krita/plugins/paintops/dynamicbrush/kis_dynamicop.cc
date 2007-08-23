/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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
#include "kis_dynamicop.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QRect>
#include <QWidget>

#include <kdebug.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_autobrush_resource.h>
#include <kis_bookmarked_configuration_manager.h>
#include <kis_bookmarked_configurations_model.h>
#include <kis_brush.h>
#include <kis_global.h>
#include <KoInputDevice.h>

#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_types.h>

#include "ui_DynamicBrushOptions.h"

#include "kis_dynamic_brush.h"
#include "kis_dynamic_brush_registry.h"
#include "kis_dynamic_coloring.h"
#include "kis_dynamic_shape.h"
#include "kis_dynamic_program.h"
#include "kis_size_transformation.h"

KisPaintOp * KisDynamicOpFactory::createOp(const KisPaintOpSettings *settings, KisPainter * painter, KisImageSP image)
{
    const KisDynamicOpSettings *dosettings = dynamic_cast<const KisDynamicOpSettings *>(settings);
    Q_ASSERT(dosettings);

    KisPaintOp * op = new KisDynamicOp(dosettings, painter);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettings *KisDynamicOpFactory::settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP image)
{
    Q_UNUSED(inputDevice);
    return new KisDynamicOpSettings(parent, m_bookmarksManager);
}

KisDynamicOpSettings::KisDynamicOpSettings(QWidget* parent, KisBookmarkedConfigurationManager* bookmarksManager) :
        QObject(parent),
        KisPaintOpSettings(parent),
        m_optionsWidget(new QWidget(parent)),
        m_uiOptions(new Ui_DynamicBrushOptions()),
        m_bookmarksModel(new KisBookmarkedConfigurationsModel(bookmarksManager))
{
    m_uiOptions->setupUi(m_optionsWidget);
    m_uiOptions->comboBoxPrograms->setModel( m_bookmarksModel );
}

KisDynamicOpSettings::~KisDynamicOpSettings()
{
    delete m_uiOptions;
}

// TEMP
#include <kis_dynamic_brush.h>
// TEMP


KisDynamicBrush* KisDynamicOpSettings::createBrush() const
{
    KisDynamicBrush* current = new KisDynamicBrush(i18n("example"));
    KisDynamicProgram* program = static_cast<KisDynamicProgram*>(m_bookmarksModel->configuration( m_uiOptions->comboBoxPrograms->view()->currentIndex() ) );
    Q_ASSERT(program);
    current->setProgram(program);
    return current;
}

KisDynamicOp::KisDynamicOp(const KisDynamicOpSettings *settings, KisPainter *painter)
    : KisPaintOp(painter), m_settings(settings)
{
    Q_ASSERT(settings);
    m_brush = m_settings->createBrush();
    m_brush->startPainting(painter);
}

KisDynamicOp::~KisDynamicOp()
{
    m_brush->endPainting();
    delete m_brush;
}

void KisDynamicOp::paintAt(const KisPaintInformation& info)
{
    KisPaintInformation adjustedInfo(info);


//     kDebug(41006) << info.pressure <<"" << info.xTilt <<"" << info.yTilt;

    // Painting should be implemented according to the following algorithm:
    // retrieve brush
    // if brush == mask
    //          retrieve mask
    // else if brush == image
    //          retrieve image
    // subsample (mask | image) for position -- pos should be double!
    // apply filters to mask (color | gradient | pattern | etc.
    // composite filtered mask into temporary layer
    // composite temporary layer into target layer
    // @see: doc/brush.txt

    if (not painter()->device()) return;

    KisPaintDeviceSP device = painter()->device();


    quint8 origOpacity = painter()->opacity();
    KoColor origColor = painter()->paintColor();

    KisDynamicShape* dabsrc = m_brush->shape()->clone();
    KisDynamicColoring* coloringsrc = m_brush->coloring()->clone();

    m_brush->program()->apply(dabsrc, coloringsrc, adjustedInfo);



    dabsrc->paintAt(info.pos, adjustedInfo, coloringsrc );


    painter()->setOpacity(origOpacity);
    painter()->setPaintColor(origColor);
}

#include "kis_dynamicop.moc"
