#ifndef KOBOOKMARKMANAGER_H
#define KOBOOKMARKMANAGER_H

#include "kotext_export.h"

#include <QList>

class KoBookmark;
class KoBookmarkManagerPrivate;

class KOTEXT_EXPORT KoBookmarkManager {
public:
    KoBookmarkManager();
    void insert(QString name, KoBookmark *bookmark);
    void remove(QString name);
    KoBookmark *retrieveBookmark(QString name);
    QList<QString> bookmarkNameList();

private:
    KoBookmarkManagerPrivate * const d;
};

#endif
