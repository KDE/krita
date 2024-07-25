class Palette : QObject
{
%TypeHeaderCode
#include "Palette.h"
%End

%ConvertToSubClassCode
  // Need to add sub class conversion, otherwise a Palette passed to a Signal is provided as a QObject instead of
  // a Palette object
  // (not sure why, but explicit conversion here fix the problem)
  Palette* mc = qobject_cast<Palette*>(sipCpp);
  sipType = 0;
  if (mc) {
      sipType = sipType_Palette;
  }
%End

    Palette(const Palette & __0);
public:
    Palette(Resource *resource);

    bool operator==(const Palette &other) const;
    bool operator!=(const Palette &other) const;

public Q_SLOTS:
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
    Swatch *entryByIndex(int index) /Factory/;
    Swatch *colorSetEntryFromGroup(int index, const QString &groupName) /Factory/;
    Swatch *entryByIndexFromGroup(int index, const QString &groupName) /Factory/;
    void addEntry(Swatch entry, QString groupName);
    void removeEntry(int index, const QString &groupName);
    void changeGroupName(QString oldGroupName, QString newGroupName);
    void renameGroup(QString oldGroupName, QString newGroupName);
    void moveGroup(const QString &groupName, const QString &groupNameInsertBefore);
    bool save();
private:
};
