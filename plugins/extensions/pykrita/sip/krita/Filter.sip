class Filter : QObject
{
%TypeHeaderCode
#include "Filter.h"
%End
    Filter(const Filter & __0);

public:
    Filter();
    virtual ~Filter();
    bool operator==(const Filter &other) const;
    bool operator!=(const Filter &other) const;

public Q_SLOTS:
    QString name() const;
    void setName(const QString &);
    InfoObject * configuration() const;
    void setConfiguration(InfoObject*  value /TransferThis/ );
    void apply(Node *node, int x, int y, int w, int h);
    bool startFilter(Node *node, int x, int y, int w, int h);
private:
};
