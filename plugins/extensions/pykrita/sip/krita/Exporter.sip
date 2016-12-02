class Exporter : QObject
{
%TypeHeaderCode
#include "Exporter.h"
%End
    Exporter(const Exporter & __0);
public:
    Exporter(QObject*  parent /TransferThis/ = 0);
    virtual ~Exporter();
    Document * document() const;
    void setDocument(Document*  value);
    InfoObject * exportSetttings() const;
    void setExportSetttings(InfoObject*  value);
public Q_SLOTS:
    bool Export(const QString & filename);
Q_SIGNALS:
private:
};
