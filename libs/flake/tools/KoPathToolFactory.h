/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHTOOLFACTORY_H
#define KOPATHTOOLFACTORY_H

#include <KoToolFactoryBase.h>

/// Factory for the KoPathTool
class KoPathToolFactory : public KoToolFactoryBase
{
public:
    KoPathToolFactory();
    ~KoPathToolFactory() override;

    KoToolBase *createTool(KoCanvasBase *canvas) override;
    QList<QAction *> createActionsImpl() override;
};

#endif
