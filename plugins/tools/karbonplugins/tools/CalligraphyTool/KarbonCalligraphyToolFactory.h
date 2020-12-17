/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KARBONCALLIGRAPHYTOOLFACTORY_H_
#define _KARBONCALLIGRAPHYTOOLFACTORY_H_

#include <KoToolFactoryBase.h>

class KarbonCalligraphyToolFactory : public KoToolFactoryBase
{
public:
    KarbonCalligraphyToolFactory();
    ~KarbonCalligraphyToolFactory() override;

    KoToolBase *createTool(KoCanvasBase *canvas) override;

    QList<QAction *> createActionsImpl() override;
};

#endif // _KARBONCALLIGRAPHYTOOLFACTORY_H_
