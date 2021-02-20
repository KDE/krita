/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISACTIONSHORTCUTSMODEL_H
#define KISACTIONSHORTCUTSMODEL_H

#include <QAbstractListModel>

class KisAbstractInputAction;
class KisInputProfile;

/**
 * \brief A QAbstractListModel subclass that lists shortcuts associated with an action from a profile.
 *
 * This class lists all shortcuts from the set profile that correspond to a specific action. This is
 * used to allow editing of shortcuts from the ui.
 *
 * It defines the following columns:
 * - Type:      The type of shortcut, can be one of Key Combination, Mouse Button, Mouse Wheel, Gesture
 * - Input:     What input is used to activate the shortcut. Depends on the type.
 * - Action:    What mode of the action will be activated by the shortcut.
 *
 * \note Before this model will provide any data you should call setAction and setProfile.
 * \note This model is editable and provides an implementation of removeRows.
 */
class KisActionShortcutsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    explicit KisActionShortcutsModel(QObject *parent = 0);
    /**
     * Destructor.
     */
    ~KisActionShortcutsModel() override;

    /**
     * Reimplemented from QAbstractItemModel::data()
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /**
     * Reimplemented from QAbstractItemModel::rowCount()
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
     * Reimplemented from QAbstractItemModel::columnCount()
     */
    int columnCount(const QModelIndex &) const override;
    /**
     * Reimplemented from QAbstractItemModel::headerData()
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role =
                                    Qt::DisplayRole) const override;
    /**
     * Reimplemented from QAbstractItemModel::flags()
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    /**
     * Reimplemented from QAbstractItemModel::setData()
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    /**
     * Reimplemented from QAbstractItemModel::removeRows.
     *
     * Removes `count` rows starting at `row`.
     *
     * \note The associated shortcuts will also be removed from the profile and completely
     * deleted.
     */
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    /**
     * \return The action used as data constraint for this model.
     */
    KisAbstractInputAction *action() const;
    /**
     * \return The profile used as data source for this model.
     */
    KisInputProfile *profile() const;

    bool canRemoveRow(int row) const;

public Q_SLOTS:
    /**
     * Set the action used as data constraint for this model.
     *
     * \param action The action to use.
     */
    void setAction(KisAbstractInputAction *action);
    /**
     * Set the profile used as data source for this model.
     *
     * \param profile The profile to get the data from.
     */
    void setProfile(KisInputProfile *profile);

private Q_SLOTS:
    void currentProfileChanged();

private:
    class Private;
    Private *const d;
};

#endif // KISACTIONSHORTCUTSMODEL_H
