/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef LOCALEHANDLER_H
#define LOCALEHANDLER_H

#include <QObject>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

/**
 * @brief The LocaleHandler class
 *
 * This object splits up a BCP47 locale into language and script sections,
 * and allows editing each.
 *
 * It also keeps track of models for script and language.
 */
class LocaleHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString bcp47Tag READ bcp47Tag WRITE setBcp47Tag NOTIFY bcp47TagChanged)
    Q_PROPERTY(QString script READ script WRITE setScript NOTIFY scriptChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

    Q_PROPERTY(QAbstractItemModel *languagesModel READ languagesModel NOTIFY languagesModelChanged)
    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)
    Q_PROPERTY(QAbstractItemModel *favoritesModel READ favoritesModel NOTIFY favoritesModelChanged)

    Q_PROPERTY(bool localeValid READ localeValid NOTIFY languageChanged)

public:
    LocaleHandler(QObject *parent = nullptr);
    ~LocaleHandler();

    QString bcp47Tag() const;
    void setBcp47Tag(const QString &newBcp47Tag);

    QString script() const;
    void setScript(const QString &newScript);

    QString language() const;
    void setLanguage(const QString &newLanguage);

    Q_INVOKABLE QVariantList scriptModel() const;

    QAbstractItemModel *languagesModel() const;

    QAbstractItemModel *favoritesModel() const;

    QString searchString() const;
    void setSearchString(const QString &newSearchString);

    bool localeValid() const;

    Q_INVOKABLE bool validBcp47Tag(const QString &tag) const;

Q_SIGNALS:

    void bcp47TagChanged();

    void scriptChanged();

    void languageChanged();

    void languagesModelChanged();

    void favoritesModelChanged();

    void searchStringChanged();

private:

    struct Private;
    const QScopedPointer<Private> d;
};

class AllLanguagesModel : public QAbstractListModel {
    Q_OBJECT
public:
    AllLanguagesModel(QObject *parent = nullptr);
    ~AllLanguagesModel();

    enum Roles {
        Code = Qt::UserRole+1
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray>  roleNames() const override;
private:
    struct Private;
    const QScopedPointer<Private> d;
};

/**
 * @brief The FavoriteLocaleModel class
 * This class keeps track of favorite locales from the config.
 */
class FavoriteLocaleModel : public QAbstractListModel {
    Q_OBJECT
public:
    FavoriteLocaleModel(QObject *parent = nullptr);
    ~FavoriteLocaleModel();

    enum Roles {
        Code = Qt::UserRole + 1,
        Favorite
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void addCode(const QString &code);

    QHash<int, QByteArray> roleNames() const override;
Q_SIGNALS:
    void favoritesUpdated();
private Q_SLOTS:
    void updateConfig();
private:


    struct Private;
    const QScopedPointer<Private> d;
};

/**
 * @brief The LanguagesFilterModel class
 * class that tests both the name and the code.
 */
class LanguagesFilterModel : public QSortFilterProxyModel {
public:
    LanguagesFilterModel(QObject *parent = nullptr);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif // LOCALEHANDLER_H
