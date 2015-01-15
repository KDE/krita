/*
 *  Copyright (c) 2014 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "animator_settings_dialog.h"
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include "kis_config.h"

AnimatorSettingsDialog::AnimatorSettingsDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setWindowTitle(i18n("Animator Settings"));
    this->setMaximumSize(300, 200);

    KisConfig cfg;
    m_autoFrameBreak = new QCheckBox(this);
    m_autoFrameBreak->setText(i18n("Enable auto frame break"));
    m_autoFrameBreak->setChecked(cfg.defAutoFrameBreakEnabled());

    QLabel* timelineWidthLbl = new QLabel(i18n("Timeline width:"), this);

    m_timelineWidth = new QSpinBox(this);
    m_timelineWidth->setMinimum(1);
    m_timelineWidth->setMaximum(10000);
    m_timelineWidth->setValue(400);

    QDialogButtonBox* buttonBox = new QDialogButtonBox();

    buttonBox->addButton(QDialogButtonBox::Ok);
    QPushButton* okButton = buttonBox->button(QDialogButtonBox::Ok);
    connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));

    buttonBox->addButton(QDialogButtonBox::Cancel);
    QPushButton* cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(m_autoFrameBreak, 0, 0);
    mainLayout->addWidget(timelineWidthLbl, 1, 0);
    mainLayout->addWidget(m_timelineWidth, 1, 1);
    mainLayout->addWidget(buttonBox, 2, 1);

    this->setLayout(mainLayout);
}

void AnimatorSettingsDialog::okClicked()
{
    m_model->enableFrameBreaking(m_autoFrameBreak->isChecked());
    emit sigTimelineWithChanged(m_timelineWidth->value());

    this->close();
}

void AnimatorSettingsDialog::cancelClicked()
{
    this->close();
}

void AnimatorSettingsDialog::setModel(KisAnimation *model)
{
    this->m_model = model;
}
