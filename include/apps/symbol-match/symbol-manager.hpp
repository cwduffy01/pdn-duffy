#pragma once

#include <map>
#include "symbol.hpp"
#include "device/drivers/serial-wrapper.hpp"
#include "utils/simple-timer.hpp"

class SymbolManager {
    public:
    SymbolManager();
    ~SymbolManager();

    Symbol* getSymbol(SerialIdentifier port);
    const char* getSymbolGlyph(SerialIdentifier port);
    void refreshSymbols();

    void validateSymbols(const uint8_t* fdnMac);
    
    void resetRefreshTimer();
    int getTimeLeftToRefresh();
    SimpleTimer* getRefreshTimer();

    /// True when the last SEND_SYMBOL on that jack had the same symbol id as this FDN for that port.
    bool isLeftOfficiallyMatched() const;
    bool isRightOfficiallyMatched() const;
    void setLeftOfficiallyMatched(bool matched);
    void setRightOfficiallyMatched(bool matched);

private:
    std::map<SerialIdentifier, Symbol*> symbols;

    SimpleTimer refreshTimer;
    int refreshInterval = (int)(10 * 1000);

    bool leftOfficiallyMatched_ = false;
    bool rightOfficiallyMatched_ = false;
};