#ifndef __koffice_kchart_h__
#define __koffice_kchart_h__

#include <ktable.h>
#include <koDocument.h>

namespace KoChart
{
    class Part;

    // the variant is either invalid (->nonexistant cell),
    // a string (interpreted as label) or a double (interpreted
    // as value)
    typedef QVariant Value;
    typedef KTable<QString,QString,Value> Data;

    class WizardExtension : public QObject
    {
        Q_OBJECT
    public:
        WizardExtension( Part *part, const char *name = 0 );
        virtual ~WizardExtension();

        Part *part() const { return m_part; }

        virtual void show() = 0;
        // XXX add more?

    private:
        Part *m_part;
        class WizardExtensionPrivate;
        WizardExtensionPrivate *d;
    };

    class Part : public KoDocument
    {
        Q_OBJECT
    public:
        Part( QWidget *parentWidget, const char *widgetName,
              QObject *parent, const char *name,
              bool singleViewMode = false );

        virtual ~Part();

        virtual void setData( const Data &d ) = 0;

        virtual WizardExtension *wizardExtension();
    private:
        class PartPrivate;
        PartPrivate *d;
    };
};

#endif
