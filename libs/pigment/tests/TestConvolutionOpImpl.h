/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _TEST_CONVOLUTION_OP_IMPL_H_
#define _TEST_CONVOLUTION_OP_IMPL_H_

#include <QObject>

class TestConvolutionOpImpl : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testConvolutionOpImpl();
    void testOneSemiTransparent();
    void testOneFullyTransparent();
};

#endif

