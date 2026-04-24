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

    bool isLeftMatched();
    bool isRightMatched();
    void setLeftMatched(bool matched);
    void setRightMatched(bool matched);

private:
    std::map<SerialIdentifier, Symbol*> symbols;

    SimpleTimer refreshTimer;
    int refreshInterval = (int)(10 * 1000);

    bool leftMatched = false;
    bool rightMatched = false;
};