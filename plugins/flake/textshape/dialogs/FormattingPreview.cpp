/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "FormattingPreview.h"

#include <KoPostscriptPaintDevice.h>
#include <KoZoomHandler.h>
#include <KoStyleThumbnailer.h>
#include <KoCharacterStyle.h>

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QString>
#include <QTextLayout>
#include <QTextLine>
#include <QTextOption>

#include <math.h>

#include <klocalizedstring.h>
#include <QDebug>

FormattingPreview::FormattingPreview(QWidget *parent)
    : QFrame(parent)
    , m_sampleText(i18n("Font"))
    , m_characterStyle(0)
    , m_paragraphStyle(0)
    , m_thumbnailer(new KoStyleThumbnailer())
    , m_previewLayoutRequired(true)
{
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setMinimumSize(500, 150);

    m_thumbnailer->setText(m_sampleText);
}

FormattingPreview::~FormattingPreview()
{
    delete m_thumbnailer;
    if (m_characterStyle) {
        delete m_characterStyle;
    }
    if (m_paragraphStyle) {
        delete m_paragraphStyle;
    }
}

void FormattingPreview::setText(const QString &sampleText)
{
    m_sampleText = sampleText;
    m_thumbnailer->setText(m_sampleText);

    m_previewLayoutRequired = true;

    update();
}

//Character properties
void FormattingPreview::setCharacterStyle(const KoCharacterStyle *style)
{
    if (m_characterStyle) {
        delete m_characterStyle;
    }

    m_characterStyle = style->clone();

    m_previewLayoutRequired = true;

    update();
}

void FormattingPreview::setParagraphStyle(const KoParagraphStyle *style)
{
    if (m_paragraphStyle) {
        delete m_paragraphStyle;
    }

    m_paragraphStyle = style->clone();

    m_previewLayoutRequired = true;

    update();
}

//Painting related methods

void FormattingPreview::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter *p = new QPainter(this);
    p->save();

    QRect rectang = contentsRect();

    p->fillRect(rectang, QBrush(QColor(Qt::white)));
    p->drawImage(rectang, m_thumbnailer->thumbnail(m_characterStyle, m_paragraphStyle, rectang.size(), m_previewLayoutRequired, KoStyleThumbnailer::NoFlags));

    m_previewLayoutRequired = false;

    p->restore();
    delete p;
}
