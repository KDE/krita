/*
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSHAPECONTROLLER_TEST_H
#define KISSHAPECONTROLLER_TEST_H

#include "kis_dummies_facade_base_test.h"

class KisDocument;
class KisNameServer;


class KisShapeControllerTest : public KisDummiesFacadeBaseTest
{
    Q_OBJECT

protected:
    KisDummiesFacadeBase* dummiesFacadeFactory() override;
    void destroyDummiesFacade(KisDummiesFacadeBase *dummiesFacade) override;

private:
    KisDocument *m_doc;
    KisNameServer *m_nameServer;
};

#endif

