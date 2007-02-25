/*
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#include <QFont>
#include <QRect>
#include <QImage>
#include <QLayout>
#include <QWidget>
#include <QString>
#include <QPixmap>
#include <QPainter>
#include <QPushButton>
#include <QFontMetrics>
#include <QLabel>

#include <khbox.h>
#include <kaction.h>
#include <kinputdialog.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kfontdialog.h>
#include <ksqueezedtextlabel.h>

#include <QColor>

#include "KoCanvasBase.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_cursor.h"
#include "kis_tool_text.h"
#include "kis_paint_device.h"
#include "KoPointerEvent.h"
#include "KoColor.h"
#include "kis_undo_adapter.h"

KisToolText::KisToolText(KoCanvasBase * canvas)
    :  KisToolPaint(canvas, KisCursor::load("tool_text_cursor.png", 6, 6))
{
    setObjectName("tool_text");
}

KisToolText::~KisToolText()
{
}

void KisToolText::mouseReleaseEvent(KoPointerEvent *e)
{
    if (m_canvas && e->button() == Qt::LeftButton) {

        bool ok;
        QString text = KInputDialog::getText(i18n("Font Tool"), i18n("Enter text:"),
             QString::null, &ok);
        if (!ok)
            return;

        KisUndoAdapter *undoAdapter = m_currentImage->undoAdapter();
        if (undoAdapter) {
            undoAdapter->beginMacro(i18n("Text"));
        }

        QFontMetrics metrics(m_font);
        QRect boundingRect = metrics.boundingRect(text).normalized();
        int xB = - boundingRect.x();
        int yB = - boundingRect.y();

        if (boundingRect.x() < 0 || boundingRect.y() < 0)
            boundingRect.translate(- boundingRect.x(), - boundingRect.y());

        QPixmap pixels(boundingRect.width(), boundingRect.height());
        {
            QPainter paint(&pixels);
            paint.fillRect(boundingRect, Qt::white);
            paint.setFont(m_font);
            paint.setBrush(QBrush(Qt::black));
            paint.drawText(xB, yB, text);
        }
        QImage image = pixels.toImage();

        qint32 height = boundingRect.height();
        qint32 width = boundingRect.width();
        KisPaintLayer *layer = new KisPaintLayer(m_currentImage.data(), '"' + text + '"', OPACITY_OPAQUE);
        KisGroupLayerSP parent = m_currentImage->rootLayer();
        if (m_currentImage->activeLayer())
            parent = m_currentImage->activeLayer()->parent();
        m_currentImage->addLayer(KisLayerSP(layer), parent, m_currentImage->activeLayer());
        QColor c = m_canvas->resourceProvider()->foregroundColor().toQColor();
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                QRgb pixel = image.pixel(x, y);
                 // use the 'blackness' as alpha :)
                quint8 alpha = 255 - qRed(pixel) * OPACITY_OPAQUE / 255;
                layer->paintDevice()->setPixel(x, y, c, alpha);
            }
        }

        layer->setOpacity(m_opacity);
        layer->setCompositeOp(m_compositeOp);
        layer->setVisible(false);

        QPoint pos = convertToPixelCoord(e).toPoint();
        qint32 x = qMax(0, static_cast<int>(pos.x() - width/2));
        qint32 y = qMax(0, static_cast<int>(pos.y() - height/2));
        layer->setX(x);
        layer->setY(y);
        layer->setVisible(true);
        layer->setDirty();

        if (undoAdapter) {
            undoAdapter->endMacro();
        }
    }
}

void KisToolText::setFont() {
    KFontDialog::getFont( m_font, false/*, QWidget* parent! */ );
    m_lbFontName->setText(QString(m_font.family() + ", %1").arg(m_font.pointSize()));
}

QWidget* KisToolText::createOptionWidget()
{
    QWidget *widget = super::createOptionWidget();

    m_lbFont = new QLabel(i18n("Font: "), widget);

    KHBox *fontBox = new KHBox(widget);
    m_lbFontName = new KSqueezedTextLabel(QString(m_font.family() + ", %1")
        .arg(m_font.pointSize()), fontBox);
    m_btnMoreFonts = new QPushButton("...", fontBox);

    connect(m_btnMoreFonts, SIGNAL(released()), this, SLOT(setFont()));

    addOptionWidgetOption(fontBox, m_lbFont);

    return widget;
}

#include "kis_tool_text.moc"
