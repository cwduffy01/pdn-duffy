#include "apps/symbol-match/symbol-manager.hpp"
#include <cstdlib>

SymbolManager::SymbolManager() {
    symbols[SerialIdentifier::INPUT_JACK] = new Symbol();
    symbols[SerialIdentifier::INPUT_JACK_SECONDARY] = new Symbol();
    refreshSymbols();
}

SymbolManager::~SymbolManager() {
    for(auto& symbol : symbols) {
        delete symbol.second;
    }
    symbols.clear();
}

void SymbolManager::refreshSymbols() {
    const SymbolId prevLeft = symbols[SerialIdentifier::INPUT_JACK]->getSymbolId();
    const SymbolId prevRight = symbols[SerialIdentifier::INPUT_JACK_SECONDARY]->getSymbolId();

    const int n = static_cast<int>(SymbolId::NUM_SYMBOLS);
    SymbolId pool[static_cast<int>(SymbolId::NUM_SYMBOLS)];
    int count = 0;
    for (int i = 0; i < n; ++i) {
        const SymbolId id = static_cast<SymbolId>(i);
        if (id != prevLeft && id != prevRight) {
            pool[count++] = id;
        }
    }

    symbols[SerialIdentifier::INPUT_JACK]->setSymbolId(pool[std::rand() % count]);
    symbols[SerialIdentifier::INPUT_JACK_SECONDARY]->setSymbolId(pool[std::rand() % count]);

    
}

Symbol* SymbolManager::getSymbol(SerialIdentifier port) {
    return symbols[port];
}

const char* SymbolManager::getSymbolGlyph(SerialIdentifier port) {
    return symbols[port]->getSymbolGlyph();
}

SimpleTimer* SymbolManager::getRefreshTimer() {
    return &refreshTimer;
}

int SymbolManager::getTimeLeftToRefresh() {
    return refreshInterval - refreshTimer.getElapsedTime();
}

void SymbolManager::resetRefreshTimer() {
    refreshTimer.setTimer(refreshInterval);
}

void SymbolManager::setLeftMatched(bool matched) {
    leftMatched = matched;
}

void SymbolManager::setRightMatched(bool matched) {
    rightMatched = matched;
}

bool SymbolManager::isLeftMatched() {
    return leftMatched;
}

bool SymbolManager::isRightMatched() {
    return rightMatched;
}