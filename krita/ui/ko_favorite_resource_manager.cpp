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
#include "kis_paintop_box.h"
#include "kis_palette_manager.h"
#include "kis_view2.h"

#ifndef _MSC_EXTENSIONS
const int KoFavoriteResourceManager::MAX_FAVORITE_BRUSHES;
//const int KoFavoriteResourceManager::MAX_RECENT_COLORS;
#endif

KoFavoriteResourceManager::KoFavoriteResourceManager(KisPaintopBox *paintopBox, QWidget* popupParent)
        :m_favoriteBrushManager(0)
        ,m_popupPalette(0)
        ,m_paintopBox(paintopBox)
        ,m_colorList(0)
{

    connect(paintopBox, SIGNAL(signalPaintopChanged(KisPaintOpPresetSP)), this, SLOT(slotChangePaintopLabel(KisPaintOpPresetSP)));

    //take favorite brushes from a file then append to QList
    KConfigGroup group(KGlobal::config(), "favoriteList");
    QStringList favoriteList = (group.readEntry("favoriteBrushes")).split(",", QString::SkipEmptyParts);

    for (int pos = 0; pos < favoriteList.size(); pos++)
    {
        KisPaintOpPresetSP newBrush = m_paintopBox->paintOpPresetSP(new KoID(favoriteList[pos], favoriteList[pos]));
        m_favoriteBrushesList.append(newBrush);
    }

    m_popupPalette = new KisPopupPalette(this, popupParent);
    m_popupPalette->setVisible(false);
    m_colorList = new KisColorDataList();
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

void KoFavoriteResourceManager::slotChangePaintopLabel(KisPaintOpPresetSP paintop)
{
    if (m_favoriteBrushManager)
        m_favoriteBrushManager->changeCurrentBrushLabel();

    //setting selected brush on pop up palette
    if (m_popupPalette)
    {
        int pos = isFavoriteBrushSaved(paintop);

        if (pos > -1) //paintop is in the list, set selected brush
        {
            m_popupPalette->setSelectedBrush(pos);
        }
        else
        {
            m_popupPalette->setSelectedBrush(-1);
        }

        m_popupPalette->update();
    }
}

//Popup Palette
void KoFavoriteResourceManager::slotShowPopupPalette(const QPoint &p)
{
    if (!m_popupPalette) return;
    else m_popupPalette->showPopupPalette(p);
}

void KoFavoriteResourceManager::resetPopupPaletteParent(QWidget* w)
{
    if (m_popupPalette)
    {
        qDebug() << "[KoFavoriteResourceManager] m_popupPalette exists and parent is being reset";
        m_popupPalette->setParent(w);
    }
}

QList<QPixmap> KoFavoriteResourceManager::favoriteBrushPixmaps()
{
    QList <QPixmap> pixmaps;

    for (int pos = 0; pos < m_favoriteBrushesList.size(); pos++)
    {
        pixmaps.append(favoriteBrushPixmap(pos));
    }
    return pixmaps;
}

QPixmap KoFavoriteResourceManager::favoriteBrushPixmap(int pos)
{
    return m_paintopBox->paintopPixmap(m_favoriteBrushesList.at(pos)->paintOp());
}

void KoFavoriteResourceManager::slotChangeActivePaintop(int pos)
{
    if (pos < 0 || pos >= m_favoriteBrushesList.size()) return;

    m_paintopBox->setCurrentPaintop(m_favoriteBrushesList.at(pos)->paintOp());

    if (m_popupPalette)
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

    int pos = isFavoriteBrushSaved(newBrush);

    if (pos > -1) //brush is saved
    {
        return pos;
    }
    else //brush hasn't been saved yet
    {
        if (isFavoriteBrushesFull())
        {
            return -2; //list is full!
        }
        else
        {
            m_favoriteBrushesList.append(newBrush);
            saveFavoriteBrushes();
            if (m_popupPalette)
            {
                m_popupPalette->setSelectedBrush(m_favoriteBrushesList.size()-1);
                m_popupPalette->update();
            }
            return -1;
        }
    }
}

int KoFavoriteResourceManager::isFavoriteBrushSaved(KisPaintOpPresetSP paintop)
{
    for (int pos = 0; pos < m_favoriteBrushesList.size(); pos ++)
    {
        if (paintop->paintOp() == m_favoriteBrushesList.at(pos)->paintOp())
            return pos;
    }

    return -1;
}

void KoFavoriteResourceManager::removeFavoriteBrush(int pos)
{
    if (pos < 0 || pos > m_favoriteBrushesList.size())
    {
        return;
    }
    else {
        m_favoriteBrushesList.removeAt(pos);
        saveFavoriteBrushes();
        if (m_popupPalette && m_popupPalette->selectedBrush() == pos) // current selected brush is deleted
        {
            m_popupPalette->setSelectedBrush(-1);
        }
        m_popupPalette->update();
    }
}

void KoFavoriteResourceManager::removeFavoriteBrush(KisPaintOpPresetSP paintop)
{
    int pos = isFavoriteBrushSaved(paintop);
    if (pos > -1) removeFavoriteBrush(pos);
}

bool KoFavoriteResourceManager::isFavoriteBrushesFull()
{
    return m_favoriteBrushesList.size() == KoFavoriteResourceManager::MAX_FAVORITE_BRUSHES;
}

int KoFavoriteResourceManager::favoriteBrushesTotal()
{
    return m_favoriteBrushesList.size();
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
    group.writeEntry("favoriteBrushes", favoriteList);
    group.config()->sync();
}

//Recent Colors
void KoFavoriteResourceManager::slotUpdateRecentColor(int pos)
{
//    qDebug() << "[KoFavoriteResourceManager] selected color: " << recentColorAt(pos)
//            << "(r)" << recentColorAt(pos).red() << "(g)" << recentColorAt(pos).green()
//            << "(b)" << recentColorAt(pos).blue();

    addRecentColorUpdate(pos);

    if (m_popupPalette)
        m_popupPalette->setVisible(false); //automatically close the palette after a button is clicked.
}

void KoFavoriteResourceManager::slotAddRecentColor(KoColor color)
{
    addRecentColor(color);
}

void KoFavoriteResourceManager::addRecentColorNew(const KoColor& color)
{
    m_colorList->appendNew(color);
    int pos = m_colorList->findPos(color);
    if (m_popupPalette)
    {
        m_popupPalette->setSelectedColor(pos);
        m_popupPalette->update();
    }
    printColors();
}

void KoFavoriteResourceManager::addRecentColorUpdate(int guipos)
{
    m_colorList->updateKey(guipos);
    if (m_popupPalette)
    {
        m_popupPalette->setSelectedColor(guipos);
        m_popupPalette->update();
    }

    emit sigSetFGColor(m_colorList->guiColor(guipos));
    printColors();
}

void KoFavoriteResourceManager::addRecentColor(const KoColor& color)
{
    m_colorList->append(color);
    int pos = m_colorList->findPos(color);
    if (m_popupPalette)
    {
        m_popupPalette->setSelectedColor(pos);
        m_popupPalette->update();
    }

     //later user can select color from the pop up palette, so it is necessary to send a signal
    emit sigSetFGColor(color);
    printColors();
}

KoFavoriteResourceManager::~KoFavoriteResourceManager()
{
    if (m_favoriteBrushManager)
        delete m_favoriteBrushManager;

    delete m_colorList;
}
#include "ko_favorite_resource_manager.moc"
