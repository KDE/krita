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

#include <qfont.h>
#include <qrect.h>
#include <qimage.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qfontmetrics.h>
#include <QLabel>

#include <khbox.h>
#include <kaction.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kfontdialog.h>
#include <ksqueezedtextlabel.h>

#include <qcolor.h>

#include "kis_point.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_cursor.h"
#include "kis_tool_text.h"
#include "kis_paint_device.h"
#include "kis_canvas_subject.h"
#include "kis_button_release_event.h"
#include "kis_color.h"
#include "kis_undo_adapter.h"

KisToolText::KisToolText()
    : super(i18n("Text"))
{
    setObjectName("tool_text");
    m_subject = 0;
    setCursor(KisCursor::load("tool_text_cursor.png", 6, 6));
}

KisToolText::~KisToolText()
{
}

void KisToolText::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(subject);
}

void KisToolText::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_subject && e->button() == Qt::LeftButton) {
        KisImageSP img = m_subject->currentImg();

        bool ok;
        QString text = KInputDialog::getText(i18n("Font Tool"), i18n("Enter text:"),
             QString::null, &ok);
        if (!ok)
            return;

        KisUndoAdapter *undoAdapter = img->undoAdapter();
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
        KisPaintLayer *layer = new KisPaintLayer(img.data(), '"' + text + '"', OPACITY_OPAQUE);
        KisGroupLayerSP parent = img->rootLayer();
        if (img->activeLayer())
            parent = img->activeLayer()->parent();
        img->addLayer(KisLayerSP(layer), parent, img->activeLayer());
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                QRgb pixel = image.pixel(x, y);
                 // use the 'blackness' as alpha :)
                quint8 alpha = 255 - qRed(pixel) * OPACITY_OPAQUE / 255;
                QColor c = m_subject->fgColor().toQColor();
                layer->paintDevice()->setPixel(x, y, c, alpha);
            }
        }

        layer->setOpacity(m_opacity);
        layer->setCompositeOp(m_compositeOp);

        qint32 x = qMax(0, static_cast<int>(e->x() - width/2));
        qint32 y = qMax(0, static_cast<int>(e->y() - height/2));
        layer->setX(x);
        layer->setY(y);

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

QWidget* KisToolText::createOptionWidget(QWidget* parent)
{
    QWidget *widget = super::createOptionWidget(parent);

    m_lbFont = new QLabel(i18n("Font: "), widget);

    KHBox *fontBox = new KHBox(widget);
    m_lbFontName = new KSqueezedTextLabel(QString(m_font.family() + ", %1")
        .arg(m_font.pointSize()), fontBox);
    m_btnMoreFonts = new QPushButton("...", fontBox);

    connect(m_btnMoreFonts, SIGNAL(released()), this, SLOT(setFont()));

    addOptionWidgetOption(fontBox, m_lbFont);

    return widget;
}

void KisToolText::setup(KActionCollection *collection)
{
    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("tool_text"),
                               i18n("T&ext"),
                               collection,
                               objectName());
        m_action->setShortcut(Qt::SHIFT+Qt::Key_T);
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setActionGroup(actionGroup());
        m_action->setToolTip(i18n("Text"));
        m_ownAction = true;
    }
}

#include "kis_tool_text.moc"
