/*
 *  dlg_perftest.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include <config.h>

#include <math.h>

#include <iostream>

using namespace std;

#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include "dlg_perftest.h"
#include "wdg_perftest.h"


DlgPerfTest::DlgPerfTest( QWidget *  parent,
                const char * name)
    : super (parent, name, true, i18n("Performance Test"), Ok | Cancel, Ok)
{
    m_lock = false;

    m_page = new WdgPerfTest(this, "perf_test");
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()),
        this, SLOT(okClicked()));

    connect(m_page->btnSelectAll, SIGNAL(clicked()), this, SLOT(selectAllClicked()));
    connect(m_page->btnDeselectAll, SIGNAL(clicked()), this, SLOT(deselectAllClicked()));
}

DlgPerfTest::~DlgPerfTest()
{
    delete m_page;
}

WdgPerfTest * DlgPerfTest::page()
{
    return m_page;
}

// SLOTS

void DlgPerfTest::okClicked()
{
    accept();
}

void DlgPerfTest::setAllTestCheckBoxes(bool checked)
{
    m_page->chkBitBlt->setChecked(checked);
    m_page->chkFill->setChecked(checked);
    m_page->chkGradient->setChecked(checked);
    m_page->chkPixel->setChecked(checked);
    m_page->chkShape->setChecked(checked);
    m_page->chkLayer->setChecked(checked);
    m_page->chkScale->setChecked(checked);
    m_page->chkRotate->setChecked(checked);
    m_page->chkRender->setChecked(checked);
    m_page->chkSelection->setChecked(checked);
    m_page->chkColorConversion->setChecked(checked);
    m_page->chkFilter->setChecked(checked);
    m_page->chkReadBytes->setChecked(checked);
    m_page->chkWriteBytes->setChecked(checked);
    m_page->chkIterators->setChecked(checked);
    m_page->chkPaintView->setChecked(checked);
    m_page->chkPaintViewFPS->setChecked(checked);
}

void DlgPerfTest::selectAllClicked()
{
    setAllTestCheckBoxes(true);
}

void DlgPerfTest::deselectAllClicked()
{
    setAllTestCheckBoxes(false);
}


#include "dlg_perftest.moc"
