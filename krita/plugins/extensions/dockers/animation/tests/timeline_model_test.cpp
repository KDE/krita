/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "timeline_model_test.h"

#include <qtest_kde.h>

#include "kis_image.h"
#include "kis_node.h"
#include "kis_paint_device.h"

#include <QDialog>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QVBoxLayout>


#include "frames_table_view.h"
#include "timeline_frames_model_testing.h"

void TimelineModelTest::testModel()
{
    QScopedPointer<TimelineFramesModelBase> model(new TimelineFramesModelTesting(0));

}

void TimelineModelTest::testView()
{
    QDialog dlg;

    QFont font;
    font.setPointSizeF(9);
    dlg.setFont(font);

    QDoubleSpinBox *dblZoom = new QDoubleSpinBox(&dlg);
    dblZoom->setValue(1.0);
    dblZoom->setSingleStep(0.1);

    QSpinBox *intFps = new QSpinBox(&dlg);
    intFps->setValue(12);

    FramesTableView *framesTable = new FramesTableView(&dlg);

    TimelineFramesModelBase *model = new TimelineFramesModelTesting(&dlg);
    framesTable->setModel(model);

    connect(dblZoom, SIGNAL(valueChanged(double)), framesTable, SLOT(setZoomDouble(double)));
    connect(intFps, SIGNAL(valueChanged(int)), framesTable, SLOT(setFramesPerSecond(int)));

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    layout->addWidget(dblZoom);
    layout->addWidget(intFps);
    layout->addWidget(framesTable);

    layout->setStretch(0, 0);
    layout->setStretch(1, 0);
    layout->setStretch(2, 1);

    dlg.resize(600, 400);

    dlg.exec();
}

QTEST_KDEMAIN(TimelineModelTest, GUI)
