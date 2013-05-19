/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
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

#ifndef KISSHAPECONTROLLER_TEST_H
#define KISSHAPECONTROLLER_TEST_H

#include "kis_dummies_facade_base_test.h"

class KisDoc2;
class KisPart2;
class KisNameServer;
class KisShapeController;


class KisShapeControllerTest : public KisDummiesFacadeBaseTest
{
    Q_OBJECT

public:
    ~KisShapeControllerTest();

protected:
    KisDummiesFacadeBase* dummiesFacadeFactory();
    void destroyDummiesFacade(KisDummiesFacadeBase *dummiesFacade);

private:
    KisDoc2 *m_doc;
    KisNameServer *m_nameServer;
};

#endif

