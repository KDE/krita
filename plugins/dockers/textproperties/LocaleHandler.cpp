/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include <text/KoWritingSystemUtils.h>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>

#include "LocaleHandler.h"

struct LocaleHandler::Private {
    Private(QObject *parent = nullptr) {
        allLanguages = new AllLanguagesModel(parent);
        filteredLanguagesModel =  new LanguagesFilterModel(parent);
        filteredLanguagesModel->setSourceModel(allLanguages);
        filteredLanguagesModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

        favorites = new FavoriteLocaleModel(parent);
    }

    KoWritingSystemUtils::Bcp47Locale locale;
    AllLanguagesModel *allLanguages = nullptr;
    LanguagesFilterModel *filteredLanguagesModel = nullptr;
    FavoriteLocaleModel *favorites = nullptr;
};

LocaleHandler::LocaleHandler(QObject *parent)
    : QObject(parent)
    , d(new Private(parent))
{

}

LocaleHandler::~LocaleHandler()
{

}

QString LocaleHandler::bcp47Tag() const
{
    return d->locale.toString();
}

void LocaleHandler::setBcp47Tag(const QString &newBcp47Tag)
{
    if (d->locale.toString() == newBcp47Tag)
        return;
    d->locale = KoWritingSystemUtils::parseBcp47Locale(newBcp47Tag);


    d->favorites->addCode(language());
    emit bcp47TagChanged();
    emit languageChanged();
    emit scriptChanged();
}

QString LocaleHandler::script() const
{
    return d->locale.scriptTag;
}

void LocaleHandler::setScript(const QString &newScript)
{
    if (d->locale.scriptTag == newScript)
        return;
    d->locale.scriptTag = newScript;
    emit scriptChanged();
    emit bcp47TagChanged();
}

QString LocaleHandler::language() const
{
    KoWritingSystemUtils::Bcp47Locale langOnly = d->locale;
    langOnly.scriptTag.clear();
    return langOnly.toString();
}

void LocaleHandler::setLanguage(const QString &newLanguage)
{
    KoWritingSystemUtils::Bcp47Locale langOnly = d->locale;
    langOnly.scriptTag.clear();
    if (langOnly.toString() == newLanguage)
        return;
    langOnly = KoWritingSystemUtils::parseBcp47Locale(newLanguage);

    d->favorites->addCode(langOnly.toString());
    langOnly.scriptTag = d->locale.scriptTag;
    d->locale = langOnly;
    emit languageChanged();
    emit bcp47TagChanged();
}

QVariantList LocaleHandler::scriptModel() const
{
    QVariantList model;
    const QString code = "code";
    const QString name = "name";

    for (int i = 0; i< int(QLocale::LastScript); i++) {
        const QLocale::Script script = QLocale::Script(i);
        QVariantMap map;
        map.insert(code, KoWritingSystemUtils::scriptTagForQLocaleScript(script));
        map.insert(name, QLocale::scriptToString(script));
        model.append(map);
    }
    return model;
}

QAbstractItemModel *LocaleHandler::languagesModel() const
{
    return d->filteredLanguagesModel;
}

QAbstractItemModel *LocaleHandler::favoritesModel() const
{
    return d->favorites;
}

QString LocaleHandler::searchString() const
{
    return d->filteredLanguagesModel->filterRegExp().pattern();
}

void LocaleHandler::setSearchString(const QString &newSearchString)
{
    if (d->filteredLanguagesModel->filterRegExp().pattern() == newSearchString)
        return;
    d->filteredLanguagesModel->setFilterFixedString(newSearchString);
    emit searchStringChanged();
}

bool LocaleHandler::localeValid() const
{
    return d->locale.isValid();
}

struct AllLanguagesModel::Private {
    Private() {
        for (int i = 0; i < QLocale::LastLanguage; i++) {
            const QLocale::Language lang = QLocale::Language(i);
            const QString langName = QLocale(lang).bcp47Name();
            if (!locales.contains(langName)) {
                locales.append(langName);
            }
            Q_FOREACH(const QLocale::Country region, QLocale::countriesForLanguage(lang)) {
                const QLocale locale(lang, region);
                const QString bcp = locale.bcp47Name().split("_").join("-");
                if (!locales.contains(bcp)) {
                    locales.append(bcp);
                }
            }
        }
    }

    QStringList locales;
};

AllLanguagesModel::AllLanguagesModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private())
{

}

AllLanguagesModel::~AllLanguagesModel()
{

}

int AllLanguagesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->locales.size();
}

QVariant AllLanguagesModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {

        const QString code = d->locales.at(index.row());
        if (role == Qt::DisplayRole) {
            KoWritingSystemUtils::Bcp47Locale bcp47 = KoWritingSystemUtils::parseBcp47Locale(code);
            const QLocale locale = KoWritingSystemUtils::localeFromBcp47Locale(bcp47);
            if (bcp47.regionTag.isEmpty()) return QLocale::languageToString(locale.language());
            return QString("%1, %2").arg(QLocale::languageToString(locale.language())).arg(QLocale::countryToString(locale.country()));
        } else if (role == Code) {
            return code;
        }
    }
    return QVariant();
}

QHash<int, QByteArray> AllLanguagesModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[Code] = "code";
    return roles;
}

struct FavoriteLocaleModel::Private {
    Private() {
        config = KSharedConfig::openConfig()->group("TextProperties");
        Q_FOREACH(const QString lang, config.readEntry<QStringList>(configKey, KLocalizedString::languages())) {
            const QLocale locale(lang);
            const QString code = locale.bcp47Name().split("_").join("-");
            locales.append(code);
        }
        if (locales.isEmpty()) {
            locales.append("");
        }
        favorites = locales;
    }

    QStringList locales;
    QStringList favorites;
    KConfigGroup config;

    const QString configKey = "favoriteLocales";
};
FavoriteLocaleModel::FavoriteLocaleModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private ())
{
}

FavoriteLocaleModel::~FavoriteLocaleModel()
{

}

int FavoriteLocaleModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->locales.size();
}

QVariant FavoriteLocaleModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        const QString code = d->locales.at(index.row());
        if (role == Qt::DisplayRole) {
            KoWritingSystemUtils::Bcp47Locale bcp47 = KoWritingSystemUtils::parseBcp47Locale(code);
            const QLocale locale = KoWritingSystemUtils::localeFromBcp47Locale(bcp47);
            if (bcp47.regionTag.isEmpty()) return QLocale::languageToString(locale.language());
            return QString("%1, %2").arg(QLocale::languageToString(locale.language())).arg(QLocale::countryToString(locale.country()));
        } else if (role == Code) {
            return code;
        } else if (role == Favorite) {
            return d->favorites.contains(code);
        }
    }
    return QVariant();
}
#include <QDebug>
bool FavoriteLocaleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;

    const QString code = d->locales.at(index.row());
    if (role == Favorite) {

        const bool fav = value.toBool();
        bool updated = false;

        if (fav) {
            if (!d->favorites.contains(code)) {
                d->favorites.append(code);
                updated = true;
            }
        } else {
            const int i = d->favorites.indexOf(code);
            if (i > 0) {
                d->favorites.removeAt(i);
                updated = true;
            }
        }
        if (updated) {
            updateConfig();
            emit dataChanged(index, index, {role});
            return true;
        }
    }
    return false;
}

void FavoriteLocaleModel::addCode(const QString &code)
{
    if (!d->locales.contains(code)) {
        beginInsertRows(QModelIndex(), d->locales.size(), d->locales.size());
        d->locales.append(code);
        endInsertRows();
    }
}

QHash<int, QByteArray> FavoriteLocaleModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[Code] = "code";
    roles[Favorite] = "favorite";
    return roles;
}

void FavoriteLocaleModel::updateConfig()
{
    d->config.writeEntry<QStringList>(d->configKey, d->favorites);
}

LanguagesFilterModel::LanguagesFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

bool LanguagesFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    const QString name = sourceModel()->data(idx).toString();
    const QString code = sourceModel()->data(idx, AllLanguagesModel::Code).toString();

    return (name.contains(filterRegExp()) || code.contains(filterRegExp()));
}


