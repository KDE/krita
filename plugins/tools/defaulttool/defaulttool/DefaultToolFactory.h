/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef DEFAULTTOOLFACTORY_H
#define DEFAULTTOOLFACTORY_H

#include <KoToolFactoryBase.h>

/// Factory for the KoInteractionTool
class DefaultToolFactory : public KoToolFactoryBase
{
public:
    /// constructor
    DefaultToolFactory();
    DefaultToolFactory(const QString &id);
    ~DefaultToolFactory() override;

    KoToolBase *createTool(KoCanvasBase *canvas) override;
    QList<QAction *> createActionsImpl() override;
};
#endif
