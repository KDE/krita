/*
 *  Copyright (c) 2003 Boudewijn Rempt
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
#include "kis_tool_paint.h"

#include <QWidget>
#include <QRect>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QWhatsThis>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QEvent>
#include <QVariant>

#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>
#include <kiconloader.h>

#include <KoShape.h>
#include <KoShapeManager.h>
#include <KoCanvasResourceProvider.h>
#include <KoColorSpace.h>
#include <KoPointerEvent.h>
#include <KoColor.h>
#include <KoCanvasBase.h>

#include <kis_types.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_paintop.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_cmb_composite.h"
#include "kis_int_spinbox.h"
#include "kis_dummy_shape.h"


KisToolPaint::KisToolPaint(KoCanvasBase * canvas, const QCursor & cursor)
    : KoTool(canvas)
    , m_cursor( cursor )
{
    m_optionWidget = 0;
    m_optionWidgetLayout = 0;

    m_lbOpacity = 0;
    m_slOpacity = 0;
    m_lbComposite= 0;
    m_cmbComposite = 0;

    m_opacity = OPACITY_OPAQUE;
    m_compositeOp = 0;
}


KisToolPaint::~KisToolPaint()
{
}


void KisToolPaint::activate(bool )
{
    emit sigCursorChanged( m_cursor );

    m_currentFgColor = m_canvas->resourceProvider()->resource( ForegroundColor ).value<KoColor>();
    m_currentBgColor = m_canvas->resourceProvider()->resource( BackgroundColor ).value<KoColor>();
    m_currentBrush = static_cast<KisBrush *>( m_canvas->resourceProvider()->resource( CurrentBrush ).value<void *>() );
    m_currentPattern = static_cast<KisPattern *>( m_canvas->resourceProvider()->resource( CurrentPattern).value<void *>() );
    m_currentGradient = static_cast<KisGradient *>( m_canvas->resourceProvider()->resource( CurrentGradient ).value<void *>() );
    m_currentPaintOp = m_canvas->resourceProvider()->resource( CurrentPaintop ).value<KoID >();
    m_currentPaintOpSettings = static_cast<KisPaintOpSettings*>( m_canvas->resourceProvider()->resource( CurrentPaintopSettings ).value<void *>() );
    m_currentLayer = m_canvas->resourceProvider()->resource( CurrentKritaLayer ).value<KisLayerSP>();
    m_currentExposure = static_cast<float>( m_canvas->resourceProvider()->resource( HdrExposure ).toDouble() );

    updateCompositeOpComboBox();
    KisConfig cfg;
    m_paintOutline = (cfg.cursorStyle() == CURSOR_STYLE_OUTLINE);

    m_currentImage = image();
}



void KisToolPaint::deactivate()
{
}

void KisToolPaint::resourceChanged( const KoCanvasResource & res )
{
    QVariant v = res.value;

    switch ( res.key ) {
    case ( ForegroundColor ):
        m_currentFgColor = v.value<KoColor>();
        break;
    case ( BackgroundColor ):
        m_currentBgColor = v.value<KoColor>();
        break;
    case ( CurrentBrush ):
        m_currentBrush = static_cast<KisBrush *>( v.value<void *>() );
        break;
    case ( CurrentPattern ):
        m_currentPattern = static_cast<KisPattern *>( v.value<void *>() );
        break;
    case ( CurrentGradient ):
        m_currentGradient = static_cast<KisGradient *>( v.value<void *>() );
        break;
    case ( CurrentPaintop ):
        m_currentPaintOp = v.value<KoID >();
        break;
    case ( CurrentPaintopSettings ):
        m_currentPaintOpSettings = static_cast<KisPaintOpSettings*>( v.value<void *>() );
        break;
    case ( CurrentKritaLayer ):
        m_currentLayer = v.value<KisLayerSP>();
        updateCompositeOpComboBox();
        break;
    case ( HdrExposure ):
        m_currentExposure = static_cast<float>( v.toDouble() );
    default:
        ;
        // Do nothing
    };
}


void KisToolPaint::paint(QPainter&, KoViewConverter &)
{
}

void KisToolPaint::mouseReleaseEvent( KoPointerEvent *e )
{
    if(e->button() == Qt::MidButton)
    {
        KoCanvasResourceProvider * resourceProvider = 0;
        if ( m_canvas && ( resourceProvider = m_canvas->resourceProvider() ) ) {
            QVariant fg = resourceProvider->resource( ForegroundColor );
            if ( !fg.isValid() ) return;
            QVariant bg = resourceProvider->resource( BackgroundColor );
            if ( !bg.isValid() ) return;
            resourceProvider->setResource( ForegroundColor, bg );
            resourceProvider->setResource( BackgroundColor, fg );
        }
    }

}

void KisToolPaint::createOptionWidget()
{
    m_optionWidget = new QWidget();

    m_lbOpacity = new QLabel(i18n("Opacity: "), m_optionWidget);
    m_slOpacity = new KisIntSpinbox( m_optionWidget, "int_m_optionwidget");
    m_slOpacity->setRange(0, 100);
    m_slOpacity->setValue(m_opacity / OPACITY_OPAQUE * 100);
    connect(m_slOpacity, SIGNAL(valueChanged(int)), this, SLOT(slotSetOpacity(int)));

    m_lbComposite = new QLabel(i18n("Mode: "), m_optionWidget);
    m_cmbComposite = new KisCmbComposite(m_optionWidget);
    connect(m_cmbComposite, SIGNAL(activated(const KoCompositeOp*)), this, SLOT(slotSetCompositeMode(const KoCompositeOp*)));

    QVBoxLayout* verticalLayout = new QVBoxLayout(m_optionWidget);
    verticalLayout->setMargin(0);
    verticalLayout->setSpacing(3);

    m_optionWidgetLayout = new QGridLayout();
    verticalLayout->addLayout(m_optionWidgetLayout);
    m_optionWidgetLayout->setSpacing(6);

    m_optionWidgetLayout->addWidget(m_lbOpacity, 0, 0);
    m_optionWidgetLayout->addWidget(m_slOpacity, 0, 1);

    m_optionWidgetLayout->addWidget(m_lbComposite, 1, 0);
    m_optionWidgetLayout->addWidget(m_cmbComposite, 1, 1);

    verticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

    if (!quickHelp().isEmpty()) {
        QPushButton* push = new QPushButton(SmallIconSet( "help" ), "", m_optionWidget);
        connect(push, SIGNAL(clicked()), this, SLOT(slotPopupQuickHelp()));

        QHBoxLayout* hLayout = new QHBoxLayout(m_optionWidget);
        hLayout->addWidget(push);
        hLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        verticalLayout->addLayout(hLayout);
    }
}


void KisToolPaint::addOptionWidgetLayout(QLayout *layout)
{
    Q_ASSERT(m_optionWidget != 0);
    Q_ASSERT(m_optionWidgetLayout != 0);
    int rowCount = m_optionWidgetLayout->rowCount();
    m_optionWidgetLayout->addLayout(layout, rowCount, 0, 1, 1);
}

void KisToolPaint::addOptionWidgetOption(QWidget *control, QWidget *label)
{
    Q_ASSERT(m_optionWidget != 0);
    Q_ASSERT(m_optionWidgetLayout != 0);
    if (label) {
        m_optionWidgetLayout->addWidget(label, m_optionWidgetLayout->rowCount(), 0);
        m_optionWidgetLayout->addWidget(control, m_optionWidgetLayout->rowCount() - 1, 1);
    }
    else
        m_optionWidgetLayout->addWidget(control, m_optionWidgetLayout->rowCount(), 0, 1, 2);
}

void KisToolPaint::slotSetOpacity(int opacityPerCent)
{
    m_opacity = opacityPerCent * OPACITY_OPAQUE / 100;
}

void KisToolPaint::slotSetCompositeMode(const KoCompositeOp* compositeOp)
{
    m_compositeOp = compositeOp;
}

KisImageSP KisToolPaint::image() const
{
    KoShapeManager * shapeManager = m_canvas->shapeManager();
    if ( !shapeManager ) return 0;

    KisDummyShape * imageShape = dynamic_cast<KisDummyShape *>( shapeManager->shapeAt(QPointF( 0, 0 )) );
    kDebug() << "Current shape: " << imageShape << endl;

    if ( !imageShape ) return 0;

    KisImageSP img = imageShape->image();
    return img;

}

void KisToolPaint::notifyModified() const
{
    KisImageSP img = image();
    if ( img ) {
        img->setModified();
    }
}

void KisToolPaint::updateCompositeOpComboBox()
{
    KisImageSP img = image();
    if (m_optionWidget && img) {
        KisPaintDeviceSP device = img->activeDevice();

        if (device) {
            KoCompositeOpList compositeOps = device->colorSpace()->userVisiblecompositeOps();
            m_cmbComposite->setCompositeOpList(compositeOps);

            if (m_compositeOp == 0 || compositeOps.indexOf(const_cast<KoCompositeOp*>(m_compositeOp)) < 0) {
                m_compositeOp = device->colorSpace()->compositeOp(COMPOSITE_OVER);
            }
            m_cmbComposite->setCurrent(m_compositeOp);
        }
    }
}

void KisToolPaint::slotPopupQuickHelp() {
    QWhatsThis::showText(QCursor::pos(), quickHelp());
}

#include "kis_tool_paint.moc"
