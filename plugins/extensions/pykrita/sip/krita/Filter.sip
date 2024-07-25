class Filter : QObject
{
%TypeHeaderCode
#include "Filter.h"
%End

%ConvertToSubClassCode
  // Need to add sub class conversion, otherwise a Filter passed to a Signal is provided as a QObject instead of
  // a Filter object
  // (not sure why, but explicit conversion here fix the problem)
  Filter* mc = qobject_cast<Filter*>(sipCpp);
  sipType = 0;
  if (mc) {
      sipType = sipType_Filter;
  }
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
