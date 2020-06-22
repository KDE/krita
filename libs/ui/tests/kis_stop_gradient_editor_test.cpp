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

#include "kis_stop_gradient_editor_test.h"

#include <QTest>
#include <QDialog>
#include <QVBoxLayout>
#include <QLinearGradient>

#include "kis_debug.h"
#include "kis_stopgradient_editor.h"


void KisStopGradientEditorTest::test()
{
    QLinearGradient gradient;
    QSharedPointer<KoStopGradient> koGradient(KoStopGradient::fromQGradient(&gradient));
    QDialog dlg;

    KisStopGradientEditor *widget = new KisStopGradientEditor(&dlg);
    widget->setGradient(koGradient);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(0,0,0,0);

    layout->addWidget(widget);
    dlg.setLayout(layout);
    dlg.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    //dlg.exec();
    qWarning() << "WARNING: showing of the dialogs in the unittest is disabled!";
}

QTEST_MAIN(KisStopGradientEditorTest)
