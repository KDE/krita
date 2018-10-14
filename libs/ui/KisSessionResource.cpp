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
#include "KisSessionResource.h"

#include <QDomElement>

#include <KisPart.h>
#include <kis_properties_configuration.h>
#include <KisDocument.h>

#include <ksharedconfig.h>
#include <KisViewManager.h>



struct KisSessionResource::Private
{
    struct View
    {
        QUuid windowId;
        QUrl file;
        KisPropertiesConfiguration viewConfig;

        KisMainWindow *getWindow() const {
            Q_FOREACH(KisMainWindow *window, KisPart::instance()->mainWindows()) {
                if (window->id() == this->windowId) return window;
            }

            return nullptr;
        }
    };
    QString profileName;
    QVector<View> views;
};

KisSessionResource::KisSessionResource(const QString &filename)
        : KisWindowLayoutResource(filename)
          , d(new Private)
{}

KisSessionResource::~KisSessionResource()
{}

void KisSessionResource::restore()
{
    auto *kisPart = KisPart::instance();

    applyLayout();

    QMap<QUrl, KisDocument *> documents;

    // Find documents which are already open so we don't need to reload them
    QList<QPointer<KisView>> oldViews = kisPart->views();
    Q_FOREACH(const QPointer<KisView> view, oldViews) {
        KisDocument *document = view->document();
        const QUrl url = document->url();
        documents.insert(url, document);
    }

    Q_FOREACH(auto &viewData, d->views) {
        QUrl url = viewData.file;

        KisMainWindow *window = viewData.getWindow();

        if (!window) {
            qDebug() << "Warning: session file contains inconsistent data.";
        } else {
            KisDocument *document = documents.value(url);

            if (!document) {
                document = kisPart->createDocument();

                bool ok = document->openUrl(url);
                if (!ok) {
                    delete document;
                    continue;
                }

                kisPart->addDocument(document);
                documents.insert(url, document);
            }
            //update profile
            QString profileName;
            profileName = d->profileName;
            window->viewManager()->changeAuthorProfile(profileName);
            window->viewManager()->slotUpdateAuthorProfileActions();


            KisView *view = window->newView(document);
            view->restoreViewState(viewData.viewConfig);
        }
    }

    Q_FOREACH(QPointer<KisView> view, oldViews) {
        view->closeView();
    }

    kisPart->setCurrentSession(this);
}


QString KisSessionResource::defaultFileExtension() const
{
    return ".ksn";
}

void KisSessionResource::storeCurrentWindows()
{
    KisPart *kisPart = KisPart::instance();
    const auto &windows = kisPart->mainWindows();
    setWindows(windows);

    d->views.clear();
    Q_FOREACH(const KisView *view, kisPart->views()) {
        if (view->document()->url().isEmpty()) continue;

        auto viewData = Private::View();
        viewData.windowId = view->mainWindow()->id();
        viewData.file = view->document()->url();
        view->saveViewState(viewData.viewConfig);
        d->views.append(viewData);
    }

    setValid(true);
}

void KisSessionResource::saveXml(QDomDocument &doc, QDomElement &root) const
{
    KisWindowLayoutResource::saveXml(doc, root);

    Q_FOREACH(const auto view, d->views) {
        QDomElement elem = doc.createElement("view");

        elem.setAttribute("window", view.windowId.toString());
        elem.setAttribute("src", view.file.toString());
        view.viewConfig.toXML(doc, elem);

        root.appendChild(elem);

        // Save profile
        KConfigGroup appAuthorGroup(KSharedConfig::openConfig(), "Author");
        QString profileName = appAuthorGroup.readEntry("active-profile", "");

        QDomElement session = doc.createElement("session");
        session.setAttribute("profile", profileName);
        root.appendChild(session);

    }

}

void KisSessionResource::loadXml(const QDomElement &root) const
{
    KisWindowLayoutResource::loadXml(root);

    d->views.clear();
    for (auto viewElement = root.firstChildElement("view");
         !viewElement.isNull();
         viewElement = viewElement.nextSiblingElement("view")) {
        Private::View view;

        view.file = QUrl(viewElement.attribute("src"));
        view.windowId = QUuid(viewElement.attribute("window"));
        view.viewConfig.fromXML(viewElement);

        d->views.append(view);
    }
    //Load session
    d->profileName.clear();
    auto sessionElement = root.firstChildElement("session");
    d->profileName = QString(sessionElement.attribute("profile"));

}
