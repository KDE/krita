/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

    explicit KeyboardModel(QObject *parent = nullptr);
    ~KeyboardModel() override;

    QHash<int, QByteArray> roleNames() const override;

    void classBegin() override;
    void componentComplete() override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

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
