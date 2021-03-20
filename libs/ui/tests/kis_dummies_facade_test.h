/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DUMMIES_FACADE_TEST_H
#define __KIS_DUMMIES_FACADE_TEST_H

#include "kis_dummies_facade_base_test.h"


class KisDummiesFacadeTest : public KisDummiesFacadeBaseTest
{
    Q_OBJECT

public:
    ~KisDummiesFacadeTest() override;

protected:
    KisDummiesFacadeBase* dummiesFacadeFactory() override;
    void destroyDummiesFacade(KisDummiesFacadeBase *dummiesFacade) override;
};

#endif /* __KIS_DUMMIES_FACADE_TEST_H */
