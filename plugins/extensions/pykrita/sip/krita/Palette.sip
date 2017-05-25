class Palette : QObject
{
%TypeHeaderCode
#include "Palette.h"
%End
    Palette(const Palette & __0);
public:
    Palette(Resource *resource);
    int columnCount();
    void setColumnCount(int columns);
    QString comment();
    QStringList groupNames();
    bool addGroup(QString name);
    bool removeGroup(QString name, bool keepColors);
    int colorsCountGroup(QString name);
private:
};
