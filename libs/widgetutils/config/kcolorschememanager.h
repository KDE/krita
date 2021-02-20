/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KCOLORSCHEMEMANAGER_H
#define KCOLORSCHEMEMANAGER_H

#include <kritawidgetutils_export.h>

#include <QObject>

class QAbstractItemModel;

class KActionMenu;
class KColorSchemeManagerPrivate;

/**
 * A small helper to get access to all available color schemes and activating a scheme in the
 * QApplication. This is useful for applications which want to provide a selection of custom color
 * schemes to their user. For example it is very common for photo and painting applications to use
 * a dark color scheme even if the default is a light scheme.
 *
 * The KColorSchemeManager provides access to a QAbstractItemModel which holds all the available
 * schemes. A possible usage looks like the following:
 *
 * @code
 * KColorSchemeManager *schemes = new KColorSchemeManager(this);
 * QListView *view = new QListView(this);
 * view->setModel(schemes->model());
 * connect(view, &QListView::activated, schemes, &KColorSchemeManager::activateScheme);
 * @endcode
 *
 * In addition the KColorSchemeManager also provides the possibility to create a KActionMenu populated
 * with all the available color schemes in an action group. If one of the actions is selected the
 * scheme is applied instantly:
 *
 * @code
 * QToolButton *button = new QToolButton();
 * KColorSchemeManager *schemes = new KColorSchemeManager(this);
 * KActionMenu *menu = schemes->createSchemeSelectionMenu(QStringLiteral("Oxygen"), button);
 * button->setMenu(menu->menu());
 * @endcode
 *
 * @since 5.0
 */
class KRITAWIDGETUTILS_EXPORT KColorSchemeManager : public QObject
{
    Q_OBJECT
public:
    explicit KColorSchemeManager(QObject *parent = 0);
    ~KColorSchemeManager() override;

    /**
     * A QAbstractItemModel of all available color schemes.
     *
     * The model provides the name of the scheme in Qt::DisplayRole, a preview
     * in Qt::DelegateRole and the full path to the scheme file in Qt::UserRole.
     *
     * @return Model of all available color schemes.
     */
    QAbstractItemModel *model() const;
    /**
     * Returns the model index for the scheme with the given @p name. If no such
     * scheme exists an invalid index is returned.
     * @see model
     */
    QModelIndex indexForScheme(const QString &name) const;

    /**
     * Creates a KActionMenu populated with all the available color schemes.
     * All actions are in an action group and when one of the actions is triggered the scheme
     * referenced by this action is activated.
     *
     * The color scheme with the same name as @p selectedSchemeName will be checked. If none
     * of the available color schemes has the same name, no action will be checked.
     *
     * The KActionMenu will not be updated in case the installed color schemes change. It's the
     * task of the user of the KActionMenu to monitor for changes if required.
     *
     * @param icon The icon to use for the KActionMenu
     * @param text The text to use for the KActionMenu
     * @param selectedSchemeName The name of the color scheme to select
     * @param parent The parent of the KActionMenu
     * @return KActionMenu populated with all available color schemes.
     * @see activateScheme
     */
    KActionMenu *createSchemeSelectionMenu(const QIcon &icon, const QString &text, const QString &selectedSchemeName, QObject *parent);
    KActionMenu *createSchemeSelectionMenu(const QString &text, const QString &selectedSchemeName, QObject *parent);
    KActionMenu *createSchemeSelectionMenu(const QString &selectedSchemeName, QObject *parent);

public Q_SLOTS:
    /**
     * @brief Activates the KColorScheme identified by the provided @p index.
     *
     * Installs the KColorScheme as the QApplication's QPalette.
     *
     * @param index The index for the KColorScheme to activate.
     * The index must reference the QAbstractItemModel provided by @c model
     * @see model
     */
    void activateScheme(const QModelIndex &index);

private:
    QScopedPointer<KColorSchemeManagerPrivate> d;
};

#endif
