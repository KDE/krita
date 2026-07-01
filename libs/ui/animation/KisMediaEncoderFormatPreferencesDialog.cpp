/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisMediaEncoderFormatPreferencesDialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <klocalizedstring.h>

#include "KisMediaEncoderWrapper.h"

KisMediaEncoderPreferencesDialog::KisMediaEncoderPreferencesDialog(KisMediaEncoderFormat *format,
                                                                   const QVariantMap &preferences,
                                                                   QWidget *parent)
    : QDialog(parent)
    , m_format(format)
{
    resize(400, 300);
    QVBoxLayout *dlgLayout = new QVBoxLayout(this);

    QDialogButtonBox *buttons;
    m_widget = format->createPreferencesWidget(preferences);
    if (m_widget) {
        dlgLayout->addWidget(m_widget, 1);
        buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
    } else {
        QLabel *noPreferencesLabel = new QLabel(i18n("This format has no preferences."));
        noPreferencesLabel->setAlignment(Qt::AlignCenter);
        noPreferencesLabel->setWordWrap(true);
        dlgLayout->addWidget(noPreferencesLabel);
        buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    }

    dlgLayout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &KisMediaEncoderPreferencesDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &KisMediaEncoderPreferencesDialog::reject);

    QPushButton *resetButton = buttons->button(QDialogButtonBox::Reset);
    if (resetButton) {
        connect(resetButton, &QPushButton::clicked, this, &KisMediaEncoderPreferencesDialog::slotReset);
    }
}

QVariant KisMediaEncoderPreferencesDialog::preferences() const
{
    return m_format->getPreferencesFromWidget(m_widget);
}

void KisMediaEncoderPreferencesDialog::slotReset()
{
    m_format->resetPreferencesWidget(m_widget);
}
