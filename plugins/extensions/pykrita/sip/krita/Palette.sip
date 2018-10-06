class KisSwatch;

class Palette : QObject
{
%TypeHeaderCode
#include "Palette.h"
%End
    Palette(const Palette & __0);
public:
    Palette(Resource *resource);
    int numberOfEntries() const;
    int columnCount();
    void setColumnCount(int columns);
    QString comment();
    void setComment(QString comment);
    QStringList groupNames();
    bool addGroup(QString name);
    bool removeGroup(QString name, bool keepColors);
    int colorsCountTotal();
    KisSwatch colorSetEntryByIndex(int index);
    KisSwatch colorSetEntryFromGroup(int index, const QString &groupName);
    ManagedColor *colorForEntry(KisSwatch entry) /Factory/;
    void addEntry(KisSwatch entry, QString groupName);
    void removeEntry(int index, const QString &groupName);
    bool changeGroupName(QString oldGroupName, QString newGroupName);
    bool moveGroup(const QString &groupName, const QString &groupNameInsertBefore);
    bool save();
private:
};
