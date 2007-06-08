#include "KoBookmarkManager.h"
#include "KoBookmark.h"

#include <KDebug>
#include <QHash>

class KoBookmarkManagerPrivate {
public:
    KoBookmarkManagerPrivate() { }
    QHash<QString, KoBookmark*> bookmarkHash;
    QList<QString> bookmarkNameList;
    int lastId;
};

KoBookmarkManager::KoBookmarkManager()
    : d(new KoBookmarkManagerPrivate)
{
}

void KoBookmarkManager::insert(QString name, KoBookmark *bookmark) {
    kDebug() << "KoBookmarkManager::insert " << name << endl;
    d->bookmarkHash[name] = bookmark;
    d->bookmarkNameList.append(name);
}

void KoBookmarkManager::remove(QString) {
}

KoBookmark *KoBookmarkManager::retrieveBookmark(QString name) {
    KoBookmark *bookmark = d->bookmarkHash.value(name);
    return bookmark;
}

QList<QString> KoBookmarkManager::bookmarkNameList() {
    return d->bookmarkNameList;
}

