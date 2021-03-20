/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_progress_widget.h"
#include <kis_debug.h>
#include <QToolButton>
#include <QHBoxLayout>
#include <QKeyEvent>

#include <kis_icon.h>

#include <KoProgressUpdater.h>
#include <KoProgressBar.h>

#include <kis_progress_updater.h>

KisProgressWidget::KisProgressWidget(QWidget* parent)
        : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    m_cancelButton = new QToolButton(this);
    m_cancelButton->setIcon(KisIconUtils::loadIcon("process-stop"));

    QSizePolicy sizePolicy = m_cancelButton->sizePolicy();
    sizePolicy.setVerticalPolicy(QSizePolicy::Ignored);
    m_cancelButton->setSizePolicy(sizePolicy);

    connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));

    m_progressBar = new KoProgressBar(this);
    // fixme:connect to the visibility changed signal if exists
    connect(m_progressBar, SIGNAL(valueChanged(int)), SLOT(correctVisibility(int)));
    layout->addWidget(m_progressBar);
    layout->addWidget(m_cancelButton);
    layout->setContentsMargins(0, 0, 0, 0);

    m_progressBar->setVisible(false);
    m_cancelButton->setVisible(false);

    setMaximumWidth(225);
    setMinimumWidth(225);
}

KisProgressWidget::~KisProgressWidget()
{
}

KoProgressProxy* KisProgressWidget::progressProxy()
{
    return m_progressBar;
}

void KisProgressWidget::cancel()
{
    Q_FOREACH (KoProgressUpdater* updater, m_activeUpdaters) {
        updater->cancel();
    }
    emit sigCancellationRequested();
}

void KisProgressWidget::correctVisibility(int progressValue)
{
    // TODO: this check duplicates code in KoProgressBar::setValue()

    const bool visibility =
        m_progressBar->minimum() == m_progressBar->maximum() ||
        (progressValue >= m_progressBar->minimum() &&
         progressValue < m_progressBar->maximum());

    m_progressBar->setVisible(visibility);
    m_cancelButton->setVisible(visibility);
}

void KisProgressWidget::detachUpdater(KoProgressUpdater* updater)
{
    m_activeUpdaters.removeOne(updater);
}

void KisProgressWidget::attachUpdater(KoProgressUpdater* updater)
{
    m_activeUpdaters << updater;
}

