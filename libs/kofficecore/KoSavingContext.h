#ifndef KOSAVINGCONTEXT_H
#define KOSAVINGCONTEXT_H

class KoGenStyles;

class KoSavingContext
{
public:    
    enum SavingMode { Store, Flat };
    
    /**
     * Constructor
     * @param mainStyles
     * @param savingMode either Store (a KoStore will be used) or Flat (all data must be inline in the XML)
     */
    explicit KoSavingContext( KoGenStyles& mainStyles, SavingMode savingMode = Store );

    ~KoSavingContext();

    KoGenStyles& mainStyles() { return m_mainStyles; }
    
    /// @return the saving mode: Store (a KoStore will be used) or Flat (all data must be inline in the XML)
    SavingMode savingMode() const { return m_savingMode; }

private:
    KoGenStyles& m_mainStyles;
    SavingMode m_savingMode;

    class Private;
    Private * const d;
};

#endif /* KOSAVINGCONTEXT_H */


