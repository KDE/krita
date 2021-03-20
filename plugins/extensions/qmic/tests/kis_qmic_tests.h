/*ls
 *  SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_QMIC_TESTS_H_
#define _KIS_QMIC_TESTS_H_

#include <simpletest.h>
#include <QImage>

#include "../gmic.h"

class KisQmicTests : public QObject
{
    Q_OBJECT

private:
    QImage m_qimage;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testConvertToGmic();
    void testConvertGrayScaleQmic();
    void testConvertGrayScaleAlphaQmic();
    void testConvertRGBqmic();
    void testConvertRGBAqmic();

};

#endif
