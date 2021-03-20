/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_stop_gradient_editor_test.h"

#include <simpletest.h>
#include <QDialog>
#include <QVBoxLayout>
#include <QLinearGradient>

#include "kis_debug.h"
#include "kis_stopgradient_editor.h"


void KisStopGradientEditorTest::test()
{
    QLinearGradient gradient;
    QSharedPointer<KoStopGradient> koGradient(KoStopGradient::fromQGradient(&gradient));
    QDialog dialog;

    KisStopGradientEditor *widget = new KisStopGradientEditor(&dialog);
    widget->setGradient(koGradient);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(0,0,0,0);

    layout->addWidget(widget);
    dialog.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    //dlg.exec();
    qWarning() << "WARNING: showing of the dialogs in the unittest is disabled!";
}

SIMPLE_TEST_MAIN(KisStopGradientEditorTest)
