class Generator : QObject
{
%TypeHeaderCode
#include "Generator.h"
%End
    Generator(const Generator & __0);
public:
    Generator(QObject*  parent /TransferThis/ = 0);
    virtual ~Generator();
    InfoObject * configuration() const;
    void setConfiguration(InfoObject*  value);
public Q_SLOTS:
    Node * CreateNode();
private:
};
