struct KoColorSetEntry {
    %TypeHeaderCode
    #include "KoColorSet.h"
    %End
public:
    KoColorSetEntry();

    QString name;
    QString id;
    bool spotColor;

    bool operator==(const KoColorSetEntry& rhs) const;

};

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
    QStringList groupNames();
    bool addGroup(QString name);
    bool removeGroup(QString name, bool keepColors);
    int colorsCountGroup(QString name);
    KoColorSetEntry colorSetEntryByIndex(int index);
    KoColorSetEntry colorSetEntryFromGroup(int index, const QString &groupName);
    ManagedColor *colorForEntry(KoColorSetEntry entry) /Factory/;
private:
};
