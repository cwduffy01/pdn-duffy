#include "apps/symbol-match/symbol-manager.hpp"
// #include <cstdlib>  // refreshSymbols: random path below (commented) used std::rand

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
    // void SymbolManager::refreshSymbols() {
    //     const SymbolId prevLeft = symbols[SerialIdentifier::INPUT_JACK]->getSymbolId();
    //     const SymbolId prevRight = symbols[SerialIdentifier::INPUT_JACK_SECONDARY]->getSymbolId();
    //
    //     const int n = static_cast<int>(SymbolId::NUM_SYMBOLS);
    //     SymbolId pool[static_cast<int>(SymbolId::NUM_SYMBOLS)];
    //     int count = 0;
    //     for (int i = 0; i < n; ++i) {
    //         const SymbolId id = static_cast<SymbolId>(i);
    //         if (id != prevLeft && id != prevRight) {
    //             pool[count++] = id;
    //         }
    //     }
    //
    //     symbols[SerialIdentifier::INPUT_JACK]->setSymbolId(pool[std::rand() % count]);
    //     symbols[SerialIdentifier::INPUT_JACK_SECONDARY]->setSymbolId(pool[std::rand() % count]);
    // }
    symbols[SerialIdentifier::INPUT_JACK]->setSymbolId(SymbolId::SYMBOL_E);
    symbols[SerialIdentifier::INPUT_JACK_SECONDARY]->setSymbolId(SymbolId::SYMBOL_D);
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

void SymbolManager::setLeftOfficiallyMatched(bool matched) {
    leftOfficiallyMatched_ = matched;
}

void SymbolManager::setRightOfficiallyMatched(bool matched) {
    rightOfficiallyMatched_ = matched;
}

bool SymbolManager::isLeftOfficiallyMatched() const {
    return leftOfficiallyMatched_;
}

bool SymbolManager::isRightOfficiallyMatched() const {
    return rightOfficiallyMatched_;
}