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
    void addGroup(QString name);
    void removeGroup(QString name, bool keepColors);
    int colorsCountTotal();
    Swatch *colorSetEntryByIndex(int index) /Factory/;
    Swatch *colorSetEntryFromGroup(int index, const QString &groupName) /Factory/;
    void addEntry(Swatch entry, QString groupName);
    void removeEntry(int index, const QString &groupName);
    void changeGroupName(QString oldGroupName, QString newGroupName);
    void moveGroup(const QString &groupName, const QString &groupNameInsertBefore);
    bool save();
private:
};
