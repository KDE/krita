class Preset : QObject
{
%TypeHeaderCode
#include "Preset.h"
%End
    Preset(const Preset & __0);
public:
    Preset(Resource *resource);
    QString toXML() const;
    void fromXML(const QString &xml);
private:
};

