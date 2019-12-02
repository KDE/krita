/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : theme manager
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

// Qt includes

#include <QObject>
#include <QPixmap>
#include <QString>

// KDE includes

#include <ksharedconfig.h>

class KActionCollection;
class KActionMenu;

namespace Digikam
{


class ThemeManager : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief ThemeManager
     * @param theme the currently active theme: the palette will not be changed to this theme
     * @param parent
     */
    explicit ThemeManager(const QString &theme = "", QObject *parent = 0);
    ~ThemeManager() override;

    QString currentThemeName() const;
    void    setCurrentTheme(const QString& name);

    void    setThemeMenuAction(KActionMenu* const action);
    void    registerThemeActions(KActionCollection *actionCollection);

Q_SIGNALS:

    void signalThemeChanged();

private Q_SLOTS:

    void slotChangePalette();

private:
    void    populateThemeMap();
    void    populateThemeMenu();
    QPixmap createSchemePreviewIcon(const KSharedConfigPtr& config);

private:

    class ThemeManagerPriv;
    ThemeManagerPriv* const d;
};

}  // namespace Digikam

#endif /* THEMEMANAGER_H */
