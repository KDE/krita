/*
 *  copyright (c) Boudewijn Rempt <boud@valdyas.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */

#ifndef KIS_PLUGIN_BASE
#define KIS_PLUGIN_BASE

/**
 * This is the base class for all Krita plugins.
 */

#include <qobject.h>

namespace Krita {

    class PluginBase : public QObject
    {

        Q_OBJECT

    public:

        PluginBase(QObject * parent, const char * name = 0) {};
        virtual ~PluginBase() {};

    };

};

#endif // KIS_PLUGIN_BASE
