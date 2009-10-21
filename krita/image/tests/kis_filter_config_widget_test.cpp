/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_filter_config_widget_test.h"

#include <qtest_kde.h>


#include "filter/kis_filter_configuration.h"
#include "kis_config_widget.h"

class TestWidget : public KisConfigWidget
{
public:
    TestWidget()
            : KisConfigWidget(0) {
    }

    void setConfiguration(const KisPropertiesConfiguration *) {
    }

    KisPropertiesConfiguration * configuration() const {
        return 0;
    }
};

void KisConfigWidgetTest::testCreation()
{
    TestWidget test;
}


QTEST_KDEMAIN(KisConfigWidgetTest, GUI)
#include "kis_filter_config_widget_test.moc"
