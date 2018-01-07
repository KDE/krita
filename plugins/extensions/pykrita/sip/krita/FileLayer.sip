class FileLayer : Node
{
%TypeHeaderCode
#include "FileLayer.h"
%End
    FileLayer(const FileLayer & __0);
public:
    virtual ~FileLayer();
    virtual QString type() const;
    void setProperties(QString FileName, QString ScalingMethod);
    QString scalingMethod() const;
    QString path() const;
Q_SIGNALS:
private:
};
