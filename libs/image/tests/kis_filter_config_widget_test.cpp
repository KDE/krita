/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filter_config_widget_test.h"

#include <simpletest.h>


#include "filter/kis_filter_configuration.h"
#include "kis_config_widget.h"

class TestWidget : public KisConfigWidget
{
public:
    TestWidget()
            : KisConfigWidget(0) {
    }

    void setConfiguration(const KisPropertiesConfigurationSP ) override {
    }

    KisPropertiesConfigurationSP  configuration() const override {
        return 0;
    }
};

void KisConfigWidgetTest::testCreation()
{
    TestWidget test;
}


SIMPLE_TEST_MAIN(KisConfigWidgetTest)
