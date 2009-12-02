/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shichan.karachu@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QDebug>
#include <QPoint>
#include <QStringList>
#include <QString>
#include <QColor>
#include <kis_paintop_registry.h>
#include <KoToolManager.h>
#include <kis_paintop_preset.h>
#include <KoID.h>
#include <ui_wdgpaintoppresets.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include "ko_favorite_resource_manager.h"
#include "kis_popup_palette.h"
#include "flowlayout.h"
#include "kis_paintop_box.h"
#include "kis_palette_manager.h"
#include "kis_view2.h"

#ifndef _MSC_EXTENSIONS
const int KoFavoriteResourceManager::MAX_FAVORITE_BRUSHES;
const int KoFavoriteResourceManager::MAX_RECENT_COLORS;
#endif

KoFavoriteResourceManager::KoFavoriteResourceManager(KisPaintopBox *paintopBox, QWidget* popupParent)
        :m_favoriteBrushManager(0)
        ,m_popupPalette(0)
        ,m_paintopBox(paintopBox)
{

    connect(paintopBox, SIGNAL(signalPaintopChanged()), this, SLOT(slotChangePaintopLabel()));

    //take favorite brushes from a file then append to QList
    KConfigGroup group(KGlobal::config(), "favoriteList");
    QStringList favoriteList = (group.readEntry("favoriteList")).split(",", QString::SkipEmptyParts);

    for (int pos = 0; pos < favoriteList.size(); pos++)
    {
        KisPaintOpPresetSP newBrush = m_paintopBox->paintOpPresetSP(new KoID(favoriteList[pos], favoriteList[pos]));
        m_favoriteBrushesList.append(newBrush);
    }

    m_popupPalette = new KisPopupPalette(this, popupParent);
    m_popupPalette->setVisible(false);
}

QStringList KoFavoriteResourceManager::favoriteBrushesStringList()
{
    QStringList list;
    for (int pos = 0; pos < m_favoriteBrushesList.size(); pos++)
    {
        list.append(m_favoriteBrushesList.at(pos)->paintOp().id());
    }

    return list;
}

void KoFavoriteResourceManager::slotChangePaintopLabel()
{
    if (m_favoriteBrushManager!=0)
        m_favoriteBrushManager->changeCurrentBrushLabel();
}

//Popup Palette
void KoFavoriteResourceManager::slotShowPopupPalette(const QPoint &p)
{
    qDebug() << "[KoFavoriteResourceManager] popup palette called";
    if (!m_popupPalette) m_popupPalette = new KisPopupPalette(this);

    m_popupPalette->showPopupPalette(p);
}

void KoFavoriteResourceManager::resetPopupPaletteParent(QWidget* w)
{
    if (m_popupPalette != 0)
    {
        qDebug() << "[KoFavoriteResourceManager] m_popupPalette exists and parent is being reset";
        m_popupPalette->setParent(w);
    }
    else
    {
        qDebug() << "[KoFavoriteResourceManager] m_popupPalette does not exist!!";
    }
}

QList<QPixmap> KoFavoriteResourceManager::favoriteBrushPixmaps()
{
    QList <QPixmap> pixmaps;

    for (int pos = 0; pos < m_favoriteBrushesList.size(); pos++)
    {
        pixmaps.append(m_paintopBox->paintopPixmap(m_favoriteBrushesList.at(pos)->paintOp()));
    }
    return pixmaps;
}

QPixmap KoFavoriteResourceManager::favoriteBrushPixmap(int pos)
{
    return m_paintopBox->paintopPixmap(m_favoriteBrushesList.at(pos)->paintOp());
}

void KoFavoriteResourceManager::slotChangeActivePaintop(int pos)
{
    qDebug() << "[KoFavoriteResourceManager] Calling brush: " << m_favoriteBrushesList.at(pos)->paintOp().id();
    if (pos < 0 || pos >= m_favoriteBrushesList.size()) return;

    m_paintopBox->setCurrentPaintop(m_favoriteBrushesList.at(pos)->paintOp());
    m_popupPalette->setVisible(false); //automatically close the palette after a button is clicked.
}

bool KoFavoriteResourceManager::isPopupPaletteVisible()
{
    if (!m_popupPalette) return false;
    else return m_popupPalette->isVisible();
}

//Palette Manager
void KoFavoriteResourceManager::showPaletteManager()
{

    if (!m_favoriteBrushManager)
    {
        m_favoriteBrushManager = new KisPaletteManager (this, m_paintopBox);

    }
    m_favoriteBrushManager->show();

}

//Favorite Brushes
int KoFavoriteResourceManager::addFavoriteBrush (KisPaintOpPresetSP newBrush)
{
    if (isFavoriteBrushesFull()) return -2;

    for (int pos = 0; pos < m_favoriteBrushesList.size(); pos ++)
    {
        if (newBrush->paintOp() == m_favoriteBrushesList.at(pos)->paintOp())
            return pos;
    }

    m_favoriteBrushesList.append(newBrush);
    saveFavoriteBrushes();

    return -1;
}

void KoFavoriteResourceManager::removeFavoriteBrush(int pos)
{
    m_favoriteBrushesList.removeAt(pos);
    saveFavoriteBrushes();
}

bool KoFavoriteResourceManager::isFavoriteBrushesFull()
{
    return m_favoriteBrushesList.size() == KoFavoriteResourceManager::MAX_FAVORITE_BRUSHES;
}

int KoFavoriteResourceManager::favoriteBrushesTotal()
{
    return m_favoriteBrushesList.size();
}

//Recent Colors
int KoFavoriteResourceManager::isInRecentColor(QColor &newColor)
{
    for (int pos=0; pos < m_recentColorsData.size(); pos++)
    {
        if (newColor.rgb() == m_recentColorsData.at(pos)->color()->rgb())
            return pos;
    }

    return -1;
}

QQueue<KisRecentColorData*> * KoFavoriteResourceManager::recentColorsList()
{
    return &(m_recentColorsData);
}

void KoFavoriteResourceManager::addRecentColor(KisRecentColorData* newColor)
{

    int pos = isInRecentColor(*(newColor->color()));
    if (pos > -1)
    {
        m_recentColorsData.removeAt(pos);
    } else {
        if (m_recentColorsData.size() == KoFavoriteResourceManager::MAX_RECENT_COLORS)
        {
            KisRecentColorData *leastUsedColor = m_recentColorsData.dequeue();
            delete leastUsedColor;
        }
    }

    m_recentColorsData.enqueue(newColor);
}

void KoFavoriteResourceManager::saveFavoriteBrushes()
{

    QString favoriteList = "";

    for (int pos = 0; pos < m_favoriteBrushesList.size(); pos++)
    {
        (favoriteList.append(m_favoriteBrushesList.at(pos)->paintOp().id())).append(",");
    }

    qDebug() << "[KoFavoriteResourceManager] Saving list: " << favoriteList;
    KConfigGroup group(KGlobal::config(), "favoriteList");    
    group.writeEntry("favoriteList", favoriteList);
    group.config()->sync();
}

KoFavoriteResourceManager::~KoFavoriteResourceManager()
{
    delete m_favoriteBrushManager;

}
#include "ko_favorite_resource_manager.moc"
