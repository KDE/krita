/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SVG_TEXT_TOOL_FACTORY
#define SVG_TEXT_TOOL_FACTORY

#include <KoToolFactoryBase.h>

class SvgTextToolFactory : public KoToolFactoryBase
{
public:
    SvgTextToolFactory();
    ~SvgTextToolFactory();

    KoToolBase *createTool(KoCanvasBase *canvas);
};

#endif
