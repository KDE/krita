/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_delayed_save_dialog.h"
#include "ui_kis_delayed_save_dialog.h"

#include <QElapsedTimer>
#include <QThread>
#include <QTimer>
#include <QtGlobal>

#include "kis_debug.h"
#include "kis_image.h"
#include "kis_composite_progress_proxy.h"

struct Q_DECL_HIDDEN KisDelayedSaveDialog::Private {
    Private(KisImageSP _image, int _busyWait, Type _type) : image(_image), busyWait(_busyWait), type(_type) {}

    KisImageSP image;
    QTimer updateTimer;
    int busyWait;

    bool checkImageIdle() {
        const bool allowLocked = type != SaveDialog;
        return image->isIdle(allowLocked);
    }

    Type type;
};

class Q_DECL_HIDDEN WdgDelayedSaveDialog : public QWidget, public Ui::KisDelayedSaveDialog
{
    Q_OBJECT

public:
    WdgDelayedSaveDialog(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

KisDelayedSaveDialog::KisDelayedSaveDialog(KisImageSP image, Type type, int busyWait, QWidget *parent)
    : KoDialog(parent)
    , ui(new WdgDelayedSaveDialog())
    , m_d(new Private(image, busyWait, type))
{
    KIS_ASSERT_RECOVER_NOOP(image);

    setMainWidget(ui);

    if (type == SaveDialog) {
        setButtons(KoDialog::Ok | KoDialog::Cancel | KoDialog::User1);
        setButtonText(KoDialog::Ok, i18n("Save without waiting"));
        setButtonText(KoDialog::Cancel, i18n("Cancel operation and save"));
        setButtonText(KoDialog::User1, i18n("Close, do not save"));

        connect(this, &KoDialog::okClicked, this, &KisDelayedSaveDialog::slotIgnoreRequested);

        connect(this, &KoDialog::cancelClicked, this, &KisDelayedSaveDialog::slotCancelRequested);

        connect(this, &KoDialog::user1Clicked, this, &KisDelayedSaveDialog::reject);
    } else if (type == GeneralDialog) {
        setButtons(KoDialog::Cancel);
        connect(this, &KoDialog::cancelClicked, &KisDelayedSaveDialog::reject);
    } else { // type == ForcedDialog, disable closing
        setButtons(KoDialog::None);
        setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    }

    connect(&m_d->updateTimer, &QTimer::timeout, this, &KisDelayedSaveDialog::slotTimerTimeout);

    m_d->image->compositeProgressProxy()->addProxy(ui->progressBar);

    resize(sizeHint());
}

KisDelayedSaveDialog::~KisDelayedSaveDialog()
{
    m_d->image->compositeProgressProxy()->removeProxy(ui->progressBar);
    delete ui;
}

void KisDelayedSaveDialog::blockIfImageIsBusy()
{
    if (m_d->checkImageIdle()) {
        setResult(Accepted);
        return;
    }

    m_d->image->requestStrokeEnd();

    QElapsedTimer t;
    t.start();

    while (t.elapsed() < m_d->busyWait) {
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        if (m_d->checkImageIdle()) {
            setResult(Accepted);
            return;
        }

        QThread::yieldCurrentThread();
    }

    m_d->updateTimer.start(200);
    exec();
    m_d->updateTimer.stop();
}

void KisDelayedSaveDialog::slotTimerTimeout()
{
    if (m_d->checkImageIdle()) {
        accept();
    }
}

void KisDelayedSaveDialog::slotCancelRequested()
{
    m_d->image->requestStrokeCancellation();
}

void KisDelayedSaveDialog::slotIgnoreRequested()
{
    done(Ignored);
}

#include "kis_delayed_save_dialog.moc"
