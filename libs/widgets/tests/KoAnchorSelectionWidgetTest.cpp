/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoAnchorSelectionWidgetTest.h"

#include <simpletest.h>
#include <QDialog>

#include <QVBoxLayout>

#include "kis_debug.h"


void KoAnchorSelectionWidgetTest::test()
{
    QDialog dlg;

    KoAnchorSelectionWidget *widget = new KoAnchorSelectionWidget(&dlg);
    connect(widget,
            SIGNAL(valueChanged(KoFlake::AnchorPosition)),
            SLOT(slotValueChanged(KoFlake::AnchorPosition)));

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->addWidget(widget);
    dlg.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    //dlg.exec();
    qWarning() << "WARNING: showing of the dialogs in the unittest is disabled!";
}

void KoAnchorSelectionWidgetTest::slotValueChanged(KoFlake::AnchorPosition id)
{
    ENTER_FUNCTION() << ppVar(id);
}

SIMPLE_TEST_MAIN(KoAnchorSelectionWidgetTest)
