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
#include <QDebug>

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kconfiggroup.h>

KisSplashScreen::KisSplashScreen(const QString &version, const QPixmap &pixmap, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, Qt::SplashScreen | Qt::FramelessWindowHint | f)
    , version(version)
    , pixmap(pixmap)
{
    setupUi(this);

    lblSplash->setPixmap(pixmap);
    bnClose->hide();
    connect(bnClose, SIGNAL(clicked()), this, SLOT(close()));
    chkShowAtStartup->hide();
    connect(chkShowAtStartup, SIGNAL(toggled(bool)), this, SLOT(toggleShowAtStartup(bool)));

    KConfigGroup cfg(KGlobal::config(), "SplashScreen");
    bool hideSplash = cfg.readEntry("HideSplashAfterStartup", false);
    chkShowAtStartup->setChecked(hideSplash);

    lblLinks->setTextFormat(Qt::RichText);
    lblLinks->setText("<html>"
                       "<head/>"
                       "<body>"
                       "<p align=\"center\"><b>Links</b></p>"
                       "<p><a href=\"http://krita.org/support-krita#general\"><span style=\" text-decoration: underline; \">Donations</span></a></p>"
                       "<p><a href=\"http://www.zazzle.com/kritashop\"><span style=\" text-decoration: underline; \">Shop</span></a></p>"
                       "<p><a href=\"http://krita.org/resources\"><span style=\" text-decoration: underline; \">Getting Started</span></a></p>"
                       "<p><a href=\"http://krita.org\"><span style=\" text-decoration: underline; \">Website</span></a></p>"
                       "<p><a href=\"http://kritastudio.com\"><span style=\" text-decoration: underline; \">Commercial Support</span></a></p>"
                       "<p><a href=\"http://forum.kde.org/viewforum.php?f=136\"><span style=\" text-decoration: underline; \">User Community</span></a></p>"
                       "<p><a href=\"https://projects.kde.org/projects/calligra\"><span style=\" text-decoration: underline; \">Source Code</span></a></p>"
                       "<p><a href=\"http://store.steampowered.com/app/280680/\"><span style=\" text-decoration: underline; \">Get Krita on Steam</span></a></p>"
                       "</body>"
                       "</html>");

    lblVersion->setText(i18n("Version: %1", version));
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
    move(QApplication::desktop()->screenGeometry().center() - r.center());
    if (isVisible()) {
        repaint();
    }

}

void KisSplashScreen::toggleShowAtStartup(bool toggle)
{
    KConfigGroup cfg(KGlobal::config(), "SplashScreen");
    cfg.writeEntry("HideSplashAfterStartup", toggle);
}
