/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KeyboardModel.h"

struct Key {
    explicit Key(QString keyText, KeyboardModel::KeyType key = KeyboardModel::NormalKey, float size = 1.0f)
        : text(keyText),
          keyType(key),
          width(size)
    {
    }

    QString text;
    KeyboardModel::KeyType keyType;
    float width;
};

class KeyboardModel::Private
{
public:
    Private()
        : mode(NormalMode),
          currentKeys(&normalKeys),
          useBuiltIn(true)
    {
#ifdef Q_OS_WIN
        useBuiltIn = false;
#endif
    }
    KeyboardMode mode;
    QList<Key> *currentKeys;
    QList<Key> normalKeys;
    QList<Key> capitalKeys;
    QList<Key> numericKeys;
    bool useBuiltIn;
};

KeyboardModel::KeyboardModel(QObject* parent)
    : QAbstractListModel(parent), d(new Private)
{
}

KeyboardModel::~KeyboardModel()
{
    delete d;
}

QHash<int, QByteArray> KeyboardModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(TextRole, "text");
    roleNames.insert(TypeRole, "keyType");
    roleNames.insert(WidthRole, "width");

    return roleNames;
}

void KeyboardModel::classBegin()
{

}

void KeyboardModel::componentComplete()
{
    //Set up keys

    //Normal mode
    //First row
    d->normalKeys.append(Key("q"));
    d->normalKeys.append(Key("w"));
    d->normalKeys.append(Key("e"));
    d->normalKeys.append(Key("r"));
    d->normalKeys.append(Key("t"));
    d->normalKeys.append(Key("y"));
    d->normalKeys.append(Key("u"));
    d->normalKeys.append(Key("i"));
    d->normalKeys.append(Key("o"));
    d->normalKeys.append(Key("p"));
    d->normalKeys.append(Key(QChar(0x2190), BackspaceKey, 2.0f));
    //Second row
    d->normalKeys.append(Key("", SpacerKey, 0.5f));
    d->normalKeys.append(Key("a"));
    d->normalKeys.append(Key("s"));
    d->normalKeys.append(Key("d"));
    d->normalKeys.append(Key("f"));
    d->normalKeys.append(Key("g"));
    d->normalKeys.append(Key("h"));
    d->normalKeys.append(Key("j"));
    d->normalKeys.append(Key("k"));
    d->normalKeys.append(Key("l"));
    d->normalKeys.append(Key("'"));
    d->normalKeys.append(Key("Enter", EnterKey, 1.5f));
    //Third row
    d->normalKeys.append(Key(QChar(0x2191), ShiftKey));
    d->normalKeys.append(Key("z"));
    d->normalKeys.append(Key("x"));
    d->normalKeys.append(Key("c"));
    d->normalKeys.append(Key("v"));
    d->normalKeys.append(Key("b"));
    d->normalKeys.append(Key("n"));
    d->normalKeys.append(Key("m"));
    d->normalKeys.append(Key(","));
    d->normalKeys.append(Key("."));
    d->normalKeys.append(Key("?"));
    d->normalKeys.append(Key(QChar(0x2191), ShiftKey));
    //Fourth row
    d->normalKeys.append(Key("&123", NumericModeKey));
    d->normalKeys.append(Key("Ctrl", SpacerKey));
    d->normalKeys.append(Key(QChar(0x263A), SpacerKey));
    d->normalKeys.append(Key(" ", NormalKey, 6.f));
    d->normalKeys.append(Key("<", LeftArrowKey));
    d->normalKeys.append(Key(">", RightArrowKey));
    d->normalKeys.append(Key("Close", CloseKey));

    //Capital mode
    //First row
    d->capitalKeys.append(Key("Q"));
    d->capitalKeys.append(Key("W"));
    d->capitalKeys.append(Key("E"));
    d->capitalKeys.append(Key("R"));
    d->capitalKeys.append(Key("T"));
    d->capitalKeys.append(Key("Y"));
    d->capitalKeys.append(Key("U"));
    d->capitalKeys.append(Key("I"));
    d->capitalKeys.append(Key("O"));
    d->capitalKeys.append(Key("P"));
    d->capitalKeys.append(Key(QChar(0x2190), BackspaceKey, 2.0f));
    //Second row
    d->capitalKeys.append(Key("", SpacerKey, 0.5f));
    d->capitalKeys.append(Key("A"));
    d->capitalKeys.append(Key("S"));
    d->capitalKeys.append(Key("D"));
    d->capitalKeys.append(Key("F"));
    d->capitalKeys.append(Key("G"));
    d->capitalKeys.append(Key("H"));
    d->capitalKeys.append(Key("J"));
    d->capitalKeys.append(Key("K"));
    d->capitalKeys.append(Key("L"));
    d->capitalKeys.append(Key("\""));
    d->capitalKeys.append(Key("Enter", EnterKey, 1.5f));
    //Third row
    d->capitalKeys.append(Key(QChar(0x2191), ShiftKey));
    d->capitalKeys.append(Key("Z"));
    d->capitalKeys.append(Key("X"));
    d->capitalKeys.append(Key("C"));
    d->capitalKeys.append(Key("V"));
    d->capitalKeys.append(Key("B"));
    d->capitalKeys.append(Key("N"));
    d->capitalKeys.append(Key("M"));
    d->capitalKeys.append(Key(";"));
    d->capitalKeys.append(Key(":"));
    d->capitalKeys.append(Key("!"));
    d->capitalKeys.append(Key(QChar(0x2191), ShiftKey));
    //Fourth row
    d->capitalKeys.append(Key("&123", NumericModeKey));
    d->capitalKeys.append(Key("Ctrl", SpacerKey));
    d->capitalKeys.append(Key(QChar(0x263A), SpacerKey));
    d->capitalKeys.append(Key(" ", NormalKey, 6.f));
    d->capitalKeys.append(Key("<", LeftArrowKey));
    d->capitalKeys.append(Key(">", RightArrowKey));
    d->capitalKeys.append(Key("Close", CloseKey));

    //Capital mode
    //First row
    d->numericKeys.append(Key("Tab", SpacerKey));
    d->numericKeys.append(Key("!"));
    d->numericKeys.append(Key("@"));
    d->numericKeys.append(Key("#"));
    d->numericKeys.append(Key("$"));
    d->numericKeys.append(Key("%"));
    d->numericKeys.append(Key("&"));
    d->numericKeys.append(Key("", SpacerKey, 0.5f));
    d->numericKeys.append(Key("1"));
    d->numericKeys.append(Key("2"));
    d->numericKeys.append(Key("3"));
    d->numericKeys.append(Key("", SpacerKey, 0.5f));
    d->numericKeys.append(Key(QChar(0x2190), BackspaceKey));
    //Second row
    d->numericKeys.append(Key("<", SpacerKey));
    d->numericKeys.append(Key("("));
    d->numericKeys.append(Key(")"));
    d->numericKeys.append(Key("-"));
    d->numericKeys.append(Key("_"));
    d->numericKeys.append(Key("="));
    d->numericKeys.append(Key("+"));
    d->numericKeys.append(Key("", SpacerKey, 0.5f));
    d->numericKeys.append(Key("4"));
    d->numericKeys.append(Key("5"));
    d->numericKeys.append(Key("6"));
    d->numericKeys.append(Key("", SpacerKey, 0.5f));
    d->numericKeys.append(Key("Enter", EnterKey));
    //Third row
    d->numericKeys.append(Key(">", SpacerKey));
    d->numericKeys.append(Key("\\"));
    d->numericKeys.append(Key(";"));
    d->numericKeys.append(Key(":"));
    d->numericKeys.append(Key("\""));
    d->numericKeys.append(Key("*"));
    d->numericKeys.append(Key("/"));
    d->numericKeys.append(Key("", SpacerKey, 0.5f));
    d->numericKeys.append(Key("7"));
    d->numericKeys.append(Key("9"));
    d->numericKeys.append(Key("9"));
    d->numericKeys.append(Key("", SpacerKey, 1.5f));
    //Fourth row
    d->numericKeys.append(Key("&123", NumericModeKey));
    d->numericKeys.append(Key("Ctrl", SpacerKey));
    d->numericKeys.append(Key(QChar(0x263A), SpacerKey));
    d->numericKeys.append(Key("<", LeftArrowKey));
    d->numericKeys.append(Key(">", RightArrowKey));
    d->numericKeys.append(Key(" ", NormalKey, 2.f));
    d->numericKeys.append(Key("", SpacerKey, 0.5f));
    d->numericKeys.append(Key("0", NormalKey, 2.f));
    d->numericKeys.append(Key("."));
    d->numericKeys.append(Key("", SpacerKey, 0.5f));
    d->numericKeys.append(Key("Close", CloseKey));
}

QVariant KeyboardModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch(role) {
        case TextRole:
            return d->currentKeys->at(index.row()).text;
        case TypeRole:
            return QVariant::fromValue<int>(d->currentKeys->at(index.row()).keyType);
        case WidthRole:
            return d->currentKeys->at(index.row()).width;
        default:
            break;
    }

    return QVariant();
}

int KeyboardModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->currentKeys->count();
}

KeyboardModel::KeyboardMode KeyboardModel::keyboardMode() const
{
    return d->mode;
}

void KeyboardModel::setKeyboardMode(KeyboardModel::KeyboardMode mode)
{
    if (mode != d->mode) {
        d->mode = mode;

        beginRemoveRows(QModelIndex(), 0, d->currentKeys->count() - 1);
        endRemoveRows();

        switch(d->mode) {
            case NormalMode:
                d->currentKeys = &d->normalKeys;
                break;
            case CapitalMode:
                d->currentKeys = &d->capitalKeys;
                break;
            case NumericMode:
                d->currentKeys = &d->numericKeys;
                break;
        }

        beginInsertRows(QModelIndex(), 0, d->currentKeys->count() - 1);
        endInsertRows();

        emit keyboardModeChanged();
    }
}

bool KeyboardModel::useBuiltIn() const
{
    return d->useBuiltIn;
}
