/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoCtlTemplatesRegistry.h"

#include <kglobal.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>

#include <kis_debug.h>
#include <OpenCTL/Template.h>
#include <qfileinfo.h>

struct KoCtlTemplatesRegistry::Private {
    QList<OpenCTL::Template*> templates;
};

KoCtlTemplatesRegistry::KoCtlTemplatesRegistry()
        : d(new Private)
{
    KGlobal::mainComponent().dirs()->addResourceType("ctl_compositeops", "data", "pigmentcms/ctlcompositeops/");
    QStringList ctlcompositeopsFilenames;
    ctlcompositeopsFilenames += KGlobal::mainComponent().dirs()->findAllResources("ctl_compositeops", "*.ctlt",  KStandardDirs::Recursive);
    dbgPlugins << "There are " << ctlcompositeopsFilenames.size() << " CTL composite ops";
    if (!ctlcompositeopsFilenames.empty()) {
        for (QStringList::Iterator it = ctlcompositeopsFilenames.begin(); it != ctlcompositeopsFilenames.end(); ++it) {
            OpenCTL::Template* ctltemplate = new OpenCTL::Template;
            ctltemplate->loadFromFile(it->toAscii().data());
            ctltemplate->compile();
            if (ctltemplate->isCompiled()) {
                dbgPlugins << "Valid composite ops template: " << *it;
                d->templates.push_back(ctltemplate);
            } else {
                dbgPlugins << "Invalid composite ops template: " << *it;
                delete ctltemplate;
            }
        }
    }
}

KoCtlTemplatesRegistry::~KoCtlTemplatesRegistry()
{
    dbgRegistry << "deleting KoCtlTemplatesRegistry";
}

const KoCtlTemplatesRegistry* KoCtlTemplatesRegistry::instance()
{
    K_GLOBAL_STATIC(KoCtlTemplatesRegistry, s_instance);
    return s_instance;

}

const QList<OpenCTL::Template*>& KoCtlTemplatesRegistry::compositeOps() const
{
    return d->templates;
}
