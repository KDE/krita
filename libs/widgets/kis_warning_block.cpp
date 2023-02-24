/*
 *  SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_warning_block.h"

#include <QHBoxLayout>
#include <QLabel>
#include <qnamespace.h>

struct Q_DECL_HIDDEN KisWarningBlock::Private {
    QLabel *lblIcon = nullptr;
    QLabel *lblText = nullptr;
};

KisWarningBlock::KisWarningBlock(QWidget *parent)
    : QGroupBox(parent)
    , m_d(new Private())
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    m_d->lblIcon = new QLabel(this);
    m_d->lblText = new QLabel(this);

    m_d->lblText->setTextFormat(Qt::RichText);
    m_d->lblIcon->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_d->lblText->setWordWrap(true);
    m_d->lblText->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);
    m_d->lblText->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    layout->addWidget(m_d->lblIcon);
    layout->addWidget(m_d->lblText);
    layout->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);

    connect(m_d->lblText, &QLabel::linkActivated, this, &KisWarningBlock::linkActivated);
}

QString KisWarningBlock::text() const
{
    return m_d->lblText->text();
}

QPixmap KisWarningBlock::pixmap() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    return m_d->lblIcon->pixmap(Qt::ReturnByValue);
#else
    return QPixmap(*m_d->lblIcon->pixmap());
#endif
}

void KisWarningBlock::setText(const QString &text)
{
    m_d->lblText->setText(text);
}

void KisWarningBlock::setPixmap(const QPixmap &icon)
{
    m_d->lblIcon->setPixmap(icon);
}

KisWarningBlock::~KisWarningBlock() = default;
