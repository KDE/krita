/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisMediaEncoderFormatPreferencesDialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "KisMediaEncoderWrapper.h"

KisMediaEncoderPreferencesDialog::KisMediaEncoderPreferencesDialog(KisMediaEncoderFormat *format,
                                                                   const QVariantMap &preferences,
                                                                   QWidget *parent)
    : QDialog(parent)
    , m_format(format)
{
    resize(400, 300);
    QVBoxLayout *dlgLayout = new QVBoxLayout(this);

    m_widget = format->createPreferencesWidget(preferences);
    dlgLayout->addWidget(m_widget, 1);

    QDialogButtonBox *buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
    dlgLayout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &KisMediaEncoderPreferencesDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &KisMediaEncoderPreferencesDialog::reject);
    connect(buttons->button(QDialogButtonBox::Reset),
            &QPushButton::clicked,
            this,
            &KisMediaEncoderPreferencesDialog::slotReset);
}

QVariant KisMediaEncoderPreferencesDialog::preferences() const
{
    return m_format->getPreferencesFromWidget(m_widget);
}

void KisMediaEncoderPreferencesDialog::slotReset()
{
    m_format->resetPreferencesWidget(m_widget);
}
