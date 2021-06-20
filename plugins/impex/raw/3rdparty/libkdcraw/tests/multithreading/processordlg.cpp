/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date  : 2014-10-17
 * @brief : test for implementation of threadWeaver api
 *
 * @author Copyright (C) 2011-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Veaceslav Munteanu
 *         <a href="mailto:veaceslav dot munteanu90 at gmail dot com">veaceslav dot munteanu90 at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "processordlg.h"

// Qt includes

#include <QList>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QDialog>
#include <QThreadPool>
#include <QFileInfo>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>
#include <QHBoxLayout>
#include <QScrollArea>

// KDE includes

#include <klocalizedstring.h>
#include <kstandardguiitem.h>

// Local includes

#include "actionthread.h"
#include "dcrawsettingswidget.h"
#include "rnuminput.h"

class ProcessorDlg::Private
{
public:

    Private()
    {
        count        = 0;
        page         = 0;
        items        = 0;
        buttons      = 0;
        progressView = 0;
        usedCore     = 0;
        thread       = 0;
        settings     = 0;
    }

    int                  count;

    QWidget*             page;
    QLabel*              items;
    QDialogButtonBox*    buttons;
    QScrollArea*         progressView;

    QList<QUrl>          list;

    DcrawSettingsWidget* settings;
    RIntNumInput*        usedCore;
    ActionThread*        thread;
};

ProcessorDlg::ProcessorDlg(const QList<QUrl>& list)
    : QDialog(qApp->activeWindow()), d(new Private)
{
    setModal(false);
    setWindowTitle(i18n("Convert RAW files To PNG"));

    d->buttons               = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close, this);
    d->thread                = new ActionThread(this);
    d->list                  = list;
    d->count                 = d->list.count();
    qDebug() << d->list;

    d->page                  = new QWidget(this);
    QVBoxLayout* const vbx   = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);

    int cpu                  = d->thread->maximumNumberOfThreads();
    QGridLayout* const grid  = new QGridLayout(d->page);
    QLabel* const pid        = new QLabel(i18n("PID: %1", QCoreApplication::applicationPid()),  this);
    QLabel* const core       = new QLabel(i18n("CPU Cores: %1", cpu), this);
    QWidget* const hbox      = new QWidget(this);
    d->items                 = new QLabel(this);

    QHBoxLayout* const hlay  = new QHBoxLayout(hbox);
    QLabel* const coresLabel = new QLabel(i18n("Cores to use: "), this);
    d->usedCore              = new RIntNumInput(this);
    d->usedCore->setRange(1, cpu, 1);
    d->usedCore->setDefaultValue(cpu);
    hlay->addWidget(coresLabel);
    hlay->addWidget(d->usedCore);
    hlay->setContentsMargins(QMargins());

    d->progressView = new QScrollArea(this);
    QWidget* const progressbox      = new QWidget(d->progressView->viewport());
    QVBoxLayout* const progressLay  = new QVBoxLayout(progressbox);
    d->progressView->setWidget(progressbox);
    d->progressView->setWidgetResizable(true);

    d->settings = new DcrawSettingsWidget(this);

    grid->addWidget(pid,             0, 0, 1, 1);
    grid->addWidget(core,            1, 0, 1, 1);
    grid->addWidget(hbox,            2, 0, 1, 1);
    grid->addWidget(d->items,        3, 0, 1, 1);
    grid->addWidget(d->progressView, 4, 0, 1, 1);
    grid->addWidget(d->settings,     0, 1, 5, 1);

    Q_FOREACH (const QUrl& url, d->list)
    {
        QProgressBar* const bar = new QProgressBar(progressbox);
        QString file            = url.toLocalFile();
        QFileInfo fi(file);
        bar->setMaximum(100);
        bar->setMinimum(0);
        bar->setValue(100);
        bar->setObjectName(file);
        bar->setFormat(fi.fileName());
        progressLay->addWidget(bar);
    }

    progressLay->addStretch();

    QPushButton* const applyBtn  = d->buttons->button(QDialogButtonBox::Apply);
    QPushButton* const cancelBtn = d->buttons->button(QDialogButtonBox::Close);

    KGuiItem::assign(applyBtn, KStandardGuiItem::apply());
    KGuiItem::assign(cancelBtn, KStandardGuiItem::cancel());

    connect(applyBtn, SIGNAL(clicked()),
            this, SLOT(slotStart()));

    connect(cancelBtn, SIGNAL(clicked()),
            this, SLOT(slotStop()));

    connect(d->thread, SIGNAL(starting(QUrl)),
            this, SLOT(slotStarting(QUrl)));

    connect(d->thread, SIGNAL(finished(QUrl)),
            this, SLOT(slotFinished(QUrl)));

    connect(d->thread, SIGNAL(failed(QUrl,QString)),
            this, SLOT(slotFailed(QUrl,QString)));

    connect(d->thread, SIGNAL(progress(QUrl,int)),
            this, SLOT(slotProgress(QUrl,int)));

    updateCount();
    resize(500, 400);
}

ProcessorDlg::~ProcessorDlg()
{
    delete d;
}

void ProcessorDlg::updateCount()
{
    d->items->setText(i18n("Files to process : %1", d->count));
}

void ProcessorDlg::slotStart()
{
    if (d->list.isEmpty()) return;

    d->buttons->button(QDialogButtonBox::Apply)->setDisabled(true);
    d->usedCore->setDisabled(true);
    d->settings->setDisabled(true);

    d->thread->setMaximumNumberOfThreads(d->usedCore->value());
    d->thread->convertRAWtoPNG(d->list, d->settings->settings());
    d->thread->start();
}

void ProcessorDlg::slotStop()
{
    d->thread->cancel();
    reject();
}

QProgressBar* ProcessorDlg::findProgressBar(const QUrl& url) const
{
    QList<QProgressBar*> bars = findChildren<QProgressBar*>();

    Q_FOREACH (QProgressBar* const b, bars)
    {
        if (b->objectName() == url.toLocalFile())
        {
            return b;
        }
    }

    qWarning() << "Cannot found relevant progress bar for " << url.toLocalFile();
    return 0;
}

void ProcessorDlg::slotStarting(const QUrl& url)
{
    qDebug() << "Start to process item " << url.toLocalFile();

    QProgressBar* const b = findProgressBar(url);

    if (b)
    {
        d->progressView->ensureWidgetVisible(b);
        b->setMinimum(0);
        b->setMaximum(100);
        b->setValue(0);
    }
}

void ProcessorDlg::slotProgress(const QUrl& url,int p)
{
    qDebug() << "Processing item " << url.toLocalFile() << " : " << p << " %";;

    QProgressBar* const b = findProgressBar(url);

    if (b)
    {
        b->setMinimum(0);
        b->setMaximum(100);
        b->setValue(p);
    }
}

void ProcessorDlg::slotFinished(const QUrl& url)
{
    qDebug() << "Completed item " << url.toLocalFile();

    QProgressBar* const b = findProgressBar(url);

    if (b)
    {
        b->setMinimum(0);
        b->setMaximum(100);
        b->setValue(100);
        b->setFormat(i18n("Done"));
        d->count--;
        updateCount();
    }
}

void ProcessorDlg::slotFailed(const QUrl& url, const QString& err)
{
    qDebug() << "Failed to complete item " << url.toLocalFile();

    QProgressBar* const b = findProgressBar(url);

    if (b)
    {
        b->setMinimum(0);
        b->setMaximum(100);
        b->setValue(100);
        b->setFormat(err);
        d->count--;
        updateCount();
    }
}
