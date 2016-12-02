class Importer : QObject
{
%TypeHeaderCode
#include "Importer.h"
%End
    Importer(const Importer & __0);
public:
    Importer(QObject*  parent /TransferThis/ = 0);
    virtual ~Importer();
    Document * document() const;
    void setDocument(Document*  value);
    InfoObject * importSettings() const;
    void setImportSettings(InfoObject*  value);
public Q_SLOTS:
    bool Import(const QString & filename);
Q_SIGNALS:
private:
};
