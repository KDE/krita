/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_elided_label.h"

struct KisElidedLabel::Private
{
    QString longText;
    Qt::TextElideMode mode;
};

KisElidedLabel::KisElidedLabel(const QString &text, Qt::TextElideMode mode, QWidget *parent)
    : QLabel(text, parent),
      m_d(new Private)
{
    m_d->mode = mode;
    m_d->longText = text;
}

KisElidedLabel::~KisElidedLabel()
{
}

void KisElidedLabel::setLongText(const QString &text)
{
    m_d->longText = text;
    updateText();
}

void KisElidedLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    updateText();
}

void KisElidedLabel::updateText()
{
    QFontMetrics metrics(font());
    QString elidedText = metrics.elidedText(m_d->longText, m_d->mode, width());
    setText(elidedText);
}
