/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CLONEOP_TEST_H
#define __KIS_CLONEOP_TEST_H

#include <simpletest.h>

class KisCloneOpTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testClone();
private:
    void testProjection();

};

#endif /* __KIS_CLONEOP_TEST_H */
