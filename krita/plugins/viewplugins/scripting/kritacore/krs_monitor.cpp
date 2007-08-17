/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Sebastian Sauer <mail@dipe.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "krs_monitor.h"
//#include <kdebug.h>
#include <kglobal.h>

// #include <QApplication>

// koffice
// #include <KoDocumentAdaptor.h>
// #include <KoApplicationAdaptor.h>
// #include <KoColorSpaceRegistry.h>

// krita
// #include <kis_autobrush_resource.h>
// #include <kis_brush.h>
// #include <kis_doc2.h>
// #include <kis_filter.h>
// #include <kis_filter_registry.h>
// #include <kis_image.h>
// #include <kis_layer.h>
// 
// #include <kis_paint_layer.h>
// #include <kis_pattern.h>
// #include <kis_resourceserver.h>
// #include <kis_view2.h>

// kritacore
// #include "krs_brush.h"
// #include "krs_color.h"
// #include "krs_filter.h"
// #include "krs_image.h"
// #include "krs_pattern.h"
// #include "krs_paint_layer.h"
// #include "krs_progress.h"

using namespace Scripting;

Monitor* Monitor::instance() {
    K_GLOBAL_STATIC(Monitor, _self)
    return _self;
}

Monitor::Monitor() : QObject() {}
Monitor::~Monitor() {}
void Monitor::started() { emit signalExecutionStarted(); }
void Monitor::finished() { emit signalExecutionFinished(); }

#include "krs_monitor.moc"
