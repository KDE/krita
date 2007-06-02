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

#include <QRect>
#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>

#include <kdebug.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_autobrush_resource.h>
#include <kis_brush.h>
#include <kis_global.h>
#include <KoInputDevice.h>
#include <kis_layer.h>
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
#include "kis_dynamic_program_registry.h"
#include "kis_size_transformation.h"

KisPaintOp * KisDynamicOpFactory::createOp(const KisPaintOpSettings *settings, KisPainter * painter, KisImageSP image)
{
    const KisDynamicOpSettings *dosettings = dynamic_cast<const KisDynamicOpSettings *>(settings);
    Q_ASSERT(dosettings);

    KisPaintOp * op = new KisDynamicOp(dosettings, painter);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettings *KisDynamicOpFactory::settings(QWidget * parent, const KoInputDevice& inputDevice)
{
    Q_UNUSED(inputDevice);
    return new KisDynamicOpSettings(parent);
}

KisDynamicOpSettings::KisDynamicOpSettings(QWidget* parent) :
        QObject(parent),
        KisPaintOpSettings(parent),
        m_optionsWidget(new QWidget(parent)),
        m_uiOptions(new Ui_DynamicBrushOptions())
{
    m_uiOptions->setupUi(m_optionsWidget);
    m_uiOptions->comboBoxPrograms->addItems( KisDynamicProgramRegistry::instance()->keys() );
}

KisDynamicOpSettings::~KisDynamicOpSettings()
{
    delete m_uiOptions;
}

// TEMP
#include <kis_dynamic_brush.h>
#include <kis_dynamic_program_registry.h>
// TEMP


KisDynamicBrush* KisDynamicOpSettings::createBrush() const
{
    KisDynamicBrush* current = new KisDynamicBrush(i18n("example"));
    KisDynamicProgram* program = KisDynamicProgramRegistry::instance()->get( m_uiOptions->comboBoxPrograms->currentText() );
    Q_ASSERT(program);
    current->setProgram(program);
    return current;
}

KisDynamicOp::KisDynamicOp(const KisDynamicOpSettings *settings, KisPainter *painter)
    : super(painter), m_settings(settings), m_dab(0)
{
    Q_ASSERT(settings);
    m_brush = m_settings->createBrush();
}

KisDynamicOp::~KisDynamicOp()
{
    delete m_brush;
}

void KisDynamicOp::paintAt(const QPointF &pos, const KisPaintInformation& info)
{
    KisPaintInformation adjustedInfo(info);


//     kDebug() << info.pressure << " " << info.xTilt << " " << info.yTilt << endl;

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

    if (not m_painter->device()) return;

    KisPaintDeviceSP device = m_painter->device();

    QPointF pt = pos;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    if(not m_dab)
    {
      m_dab = new KisPaintDevice(device->colorSpace());
    }

    quint8 origOpacity = m_painter->opacity();
    KoColor origColor = m_painter->paintColor();

    KisDynamicShape* dabsrc = m_brush->shape()->clone();
    KisDynamicColoring* coloringsrc = m_brush->coloring()->clone();

    m_brush->program()->apply(dabsrc, coloringsrc, adjustedInfo);
    
    dabsrc->createStamp(m_dab, coloringsrc, pos, adjustedInfo);

    // paint the dab
    QRect dabRect = dabsrc->rect();
    QRect dstRect = QRect(x + dabRect.x(), y + dabRect.y(), dabRect.width(), dabRect.height());

    if ( m_painter->bounds().isValid() ) {
        dstRect &= m_painter->bounds();
    }

    if (dstRect.isNull() or dstRect.isEmpty() or not dstRect.isValid()) return;

    qint32 sx = dabRect.x() ;//dstRect.x() - x;
    qint32 sy = dabRect.y();//dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();
//     kDebug() << sx << " " << sy << " " << sw << " " << sh << endl;
    if (m_source->hasSelection()) {
        m_painter->bltSelection(dstRect.x(), dstRect.y(), m_painter->compositeOp(), m_dab,
                                m_source->selection(), m_painter->opacity(), sx, sy, sw, sh);
    }
    else {
        m_painter->bitBlt(dstRect.x(), dstRect.y(), m_painter->compositeOp(), m_dab, m_painter->opacity(), sx, sy, sw, sh);
    }

    m_painter->setOpacity(origOpacity);
    m_painter->setPaintColor(origColor);
}

#include "kis_dynamicop.moc"
