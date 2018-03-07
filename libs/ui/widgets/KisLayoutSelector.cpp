/*
 *  Copyright (c) 2018 Jouni Pentikäinen <joupent@gmail.com>
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

#include <KisWindowLayoutResource.h>
#include <kis_resource_server_provider.h>
#include <KisWindowLayoutManager.h>
#include "KisLayoutSelector.h"
#include "KisSessionResource.h"

struct KisLayoutSelector::Private {
    enum ItemTypes {
        WindowLayout,
        Session
    };

    KoResourceServer<KisWindowLayoutResource> * windowLayoutServer;
    KoResourceServer<KisSessionResource> * sessionServer;

    bool blockChange = false;
};

KisLayoutSelector::KisLayoutSelector(QWidget *parent)
    : QComboBox(parent)
    , d(new Private)
{
    d->windowLayoutServer = KisResourceServerProvider::instance()->windowLayoutServer(false);
    d->sessionServer = KisResourceServerProvider::instance()->sessionServer(false);

    connect(this, SIGNAL(activated(int)), this, SLOT(slotItemChanged(int)));

    updateItems();
}

KisLayoutSelector::~KisLayoutSelector() {}

void KisLayoutSelector::updateItems() {
    d->blockChange = true;

    clear();

    Q_FOREACH(KisWindowLayoutResource *windowLayout, d->windowLayoutServer->resources()) {
        addItem(windowLayout->name(), Private::WindowLayout);
    }

    Q_FOREACH(KisSessionResource *session, d->sessionServer->resources()) {
        addItem(session->name(), Private::Session);
    }

    const QString &lastLayout = KisWindowLayoutManager::instance()->lastLayoutName();
    if (!lastLayout.isEmpty()) {
        int index = findText(lastLayout);
        if (index >=0) setCurrentIndex(index);
    }

    d->blockChange = false;
}

void KisLayoutSelector::slotItemChanged(int index)
{
    if (index < 0) return;
    if (d->blockChange) return;

    const QString name = itemText(index);

    switch (currentData(Qt::UserRole).toInt()) {
        case Private::WindowLayout: {
            KisWindowLayoutResource *windowLayout = d->windowLayoutServer->resourceByName(name);
            if (windowLayout) {
                windowLayout->applyLayout();
            }
        } break;
        case Private::Session: {
            KisSessionResource *session = d->sessionServer->resourceByName(name);
            if (session) {
                session->restore();
            }
        } break;
        default:
            qDebug() << "Unexpected item type in KisLayoutSelector.";
            break;
    }
}