/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#ifndef KEYBOARDMODEL_H
#define KEYBOARDMODEL_H

#include <QAbstractListModel>
#include <QQmlParserStatus>

class KeyboardModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(KeyboardMode mode READ keyboardMode WRITE setKeyboardMode NOTIFY keyboardModeChanged)
    Q_PROPERTY(bool useBuiltIn READ useBuiltIn NOTIFY useBuiltInChanged)

public:
    enum Roles {
        TextRole = Qt::UserRole + 1,
        TypeRole,
        WidthRole
    };

    enum KeyboardMode {
        NormalMode,
        CapitalMode,
        NumericMode
    };
    Q_ENUMS(KeyboardMode)

    enum KeyType {
        NormalKey,
        SpacerKey,
        ShiftKey,
        EnterKey,
        BackspaceKey,
        NumericModeKey,
        CloseKey,
        LeftArrowKey,
        RightArrowKey
    };
    Q_ENUMS(KeyType)

    explicit KeyboardModel(QObject* parent = 0);
    virtual ~KeyboardModel();

    virtual void classBegin();
    virtual void componentComplete();

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    KeyboardMode keyboardMode() const;
    void setKeyboardMode(KeyboardModel::KeyboardMode mode);

    bool useBuiltIn() const;

Q_SIGNALS:
    void keyboardModeChanged();
    bool useBuiltInChanged();

private:
    class Private;
    Private * const d;
};

#endif // KEYBOARDMODEL_H
