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

#ifndef KISWINDOWLAYOUTMANAGER_H
#define KISWINDOWLAYOUTMANAGER_H

#include <QObject>
#include <QUuid>
#include <QVector>
#include <QSize>

#include <KisWindowLayoutResource.h>

class QScreen;
class KisDocument;

class KisWindowLayoutManager : public QObject
{
    Q_OBJECT

public:
    struct Display
    {
        QSize resolution;

        bool matches(QScreen* screen) const;
    };

    struct DisplayLayout
    {
        QString name;

        QVector<Display> displays;
        QString preferredWindowLayout;

        bool matches(QList<QScreen*> screens) const;
    };

    explicit KisWindowLayoutManager();
    ~KisWindowLayoutManager();

    static KisWindowLayoutManager *instance();

    /**
     * When enabled, a workspace dedicated as primary is used for any main window which receives focus.
     * Meanwhile, the workspace of that window is used for the window which originally had the primary workspace.
     */
    bool primaryWorkspaceFollowsFocus() const;
    void setPrimaryWorkspaceFollowsFocus(bool enabled, QUuid primaryWindow);
    QUuid primaryWindowId() const;

    /**
     * When enabled, main windows will synchronize to keep the same document active
     */
    bool isShowImageInAllWindowsEnabled() const;
    void setShowImageInAllWindowsEnabled(bool showInAll);

    void activeDocumentChanged(KisDocument *document);

    void setLastUsedLayout(KisWindowLayoutResource *layout);

private Q_SLOTS:
    void slotFocusChanged(QWidget*, QWidget*);
    void slotScreensChanged();

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif
