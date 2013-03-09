/*
 *  Copyright (c) 2013 Marijn Kruisselbrink <mkruisselbrink@kde.org>
 *  Copyright (C) 2012-2013 Jigar Raisinghani <jigarraisinghani@gmail.com>
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

#include "pivotplugin.h"

#include <klocale.h>
#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kurl.h>
#include <kpluginfactory.h>
#include <kmessagebox.h>

#include <sheets/part/Canvas.h>
#include <sheets/part/View.h>
#include <sheets/ui/Selection.h>

#include "pivot.h"

using namespace Calligra::Sheets;

K_PLUGIN_FACTORY(PivotPluginFactory, registerPlugin<PivotPlugin>();)
K_EXPORT_PLUGIN(PivotPluginFactory("sheetspivottables_plugin"))

PivotPlugin::PivotPlugin(QObject *parent, const QVariantList &)
    : KParts::Plugin(parent)
{
    setComponentData(PivotPluginFactory::componentData());

    KAction *action = new KAction(i18n("&Pivot..."), this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_P));
    actionCollection()->addAction("pivot", action );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(pivot()));
    action->setToolTip(i18n("Pivot Tables"));
}

PivotPlugin::~PivotPlugin()
{
}

void PivotPlugin::pivot()
{
    View *view = dynamic_cast<View *>(parent());
    if (!view) {
        return;
    }

    if ((view->selection()->lastRange().width() < 2) || (view->selection()->lastRange().height() < 2)) {
        KMessageBox::error(view->canvasWidget(), i18n("You must select multiple cells."));
        return;
    }

    QPointer<Pivot> dialog = new Pivot(view->canvasWidget(), view->selection());
    dialog->exec();
    delete dialog;
}
