/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_AUTO_BRUSH_FACTORY_TEST_H
#define KIS_AUTO_BRUSH_FACTORY_TEST_H

#include <simpletest.h>

class KisAutoBrushFactoryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testXMLClone();
};

#endif // KIS_AUTO_BRUSH_FACTORY_TEST_H
