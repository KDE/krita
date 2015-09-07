/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_splash_screen.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QCheckBox>
#include <kis_debug.h>
#include <QFile>

#include <KisPart.h>
#include <KisApplication.h>

#include <KoIcon.h>

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <kcomponentdata.h>
#include <kicon.h>

KisSplashScreen::KisSplashScreen(const QString &version, const QPixmap &pixmap, bool themed, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, Qt::SplashScreen | Qt::FramelessWindowHint | f)
{
    setupUi(this);
    setWindowIcon(koIcon("calligrakrita"));

    QString color = "#FFFFFF";
    if (themed && qApp->palette().background().color().value() >100) {
        color = "#000000";
    }


    lblSplash->setPixmap(pixmap);
    bnClose->hide();
    connect(bnClose, SIGNAL(clicked()), this, SLOT(close()));
    chkShowAtStartup->hide();
    connect(chkShowAtStartup, SIGNAL(toggled(bool)), this, SLOT(toggleShowAtStartup(bool)));

    KConfigGroup cfg(KGlobal::config(), "SplashScreen");
    bool hideSplash = cfg.readEntry("HideSplashAfterStartup", false);
    chkShowAtStartup->setChecked(hideSplash);

    lblLinks->setTextFormat(Qt::RichText);
    lblLinks->setText(i18n("<html>"
                           "<head/>"
                           "<body>"
                           "<p align=\"center\"><span style=\" color:%1;\"><b>Links</b></span></p>"

                           "<p><a href=\"https://krita.org/support-us/donations/\"><span style=\" text-decoration: underline; color:%1;\">Support Krita</span></a></p>"

                           "<p><a href=\"http://krita.org/resources\"><span style=\" text-decoration: underline; color:%1;\">Getting Started</span></a></p>"
                           "<p><a href=\"http://userbase.kde.org/Krita/Manual\"><span style=\" text-decoration: underline; color:%1;\">Manual</span></a></p>"
                           "<p><a href=\"http://krita.org\"><span style=\" text-decoration: underline; color:%1;\">Krita Website</span></a></p>"
                           "<p><a href=\"http://forum.kde.org/viewforum.php?f=136\"><span style=\" text-decoration: underline; color:%1;\">User Community</span></a></p>"

                           "<p><a href=\"https://projects.kde.org/projects/calligra\"><span style=\" text-decoration: underline; color:%1;\">Source Code</span></a></p>"

                           "<p><a href=\"http://store.steampowered.com/app/280680/\"><span style=\" text-decoration: underline; color:%1;\">Krita on Steam</span></a></p>"
                           "</body>"
                           "</html>").arg(color));

    lblVersion->setText(i18n("Version: %1", version));
    lblVersion->setStyleSheet("color:" + color);

    KConfigGroup cfg2(KGlobal::config(), "RecentFiles");
    int i = 1;

    QString recent = i18n("<html>"
                          "<head/>"
                          "<body>"
                          "<p align=\"center\"><b><span style=\" color:%1;\">Recent Files</span></b></p>").arg(color);

    QString path;
    QStringList recentfiles;

    do {
        path = cfg2.readPathEntry(QString("File%1").arg(i), QString());
        if (!path.isEmpty()) {
            QString name = cfg2.readPathEntry(QString("Name%1").arg(i), QString());
            KUrl url(path);
            if (name.isEmpty())
                name = url.fileName();

            if (!url.isLocalFile() || QFile::exists(url.toLocalFile())) {
                recentfiles.insert(0, QString("<p><a href=\"%1\"><span style=\"color:%3;\">%2</span></a></p>").arg(path).arg(name).arg(color));
            }
        }

        i++;
    } while (!path.isEmpty() || i <= 8);

    recent += recentfiles.join("\n");
    recent += "</body>"
            "</html>";
    lblRecent->setText(recent);
    connect(lblRecent, SIGNAL(linkActivated(QString)), SLOT(linkClicked(QString)));
}


void KisSplashScreen::repaint()
{
    QWidget::repaint();
    QApplication::flush();
}

void KisSplashScreen::show()
{
    QRect r(QPoint(), sizeHint());
    resize(r.size());
    move(QApplication::desktop()->availableGeometry().center() - r.center());
    if (isVisible()) {
        repaint();
    }

}

void KisSplashScreen::toggleShowAtStartup(bool toggle)
{
    KConfigGroup cfg(KGlobal::config(), "SplashScreen");
    cfg.writeEntry("HideSplashAfterStartup", toggle);
}

void KisSplashScreen::linkClicked(const QString &link)
{
    KisPart::instance()->openExistingFile(KUrl(link));
    if (isTopLevel()) {
        close();
    }
}
