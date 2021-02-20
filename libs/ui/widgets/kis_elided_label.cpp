/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
