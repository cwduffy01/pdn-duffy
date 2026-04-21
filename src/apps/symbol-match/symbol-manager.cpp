#include "apps/symbol-match/symbol-manager.hpp"
#include <cstdlib>

SymbolManager::SymbolManager() {
    symbols[SymbolPosition::LEFT] = new Symbol();
    symbols[SymbolPosition::RIGHT] = new Symbol();
    refreshSymbols();
}

SymbolManager::~SymbolManager() {
    for(auto& symbol : symbols) {
        delete symbol.second;
    }
    symbols.clear();
}

void SymbolManager::refreshSymbols() {
    const SymbolId prevLeft = symbols[SymbolPosition::LEFT]->getSymbolId();
    const SymbolId prevRight = symbols[SymbolPosition::RIGHT]->getSymbolId();

    const int n = static_cast<int>(SymbolId::NUM_SYMBOLS);
    SymbolId pool[static_cast<int>(SymbolId::NUM_SYMBOLS)];
    int count = 0;
    for (int i = 0; i < n; ++i) {
        const SymbolId id = static_cast<SymbolId>(i);
        if (id != prevLeft && id != prevRight) {
            pool[count++] = id;
        }
    }

    symbols[SymbolPosition::LEFT]->setSymbolId(pool[std::rand() % count]);
    symbols[SymbolPosition::RIGHT]->setSymbolId(pool[std::rand() % count]);
}

Symbol* SymbolManager::getSymbol(SymbolPosition position) {
    return symbols[position];
}

const char* SymbolManager::getSymbolGlyph(SymbolPosition position) {
    return symbols[position]->getSymbolGlyph();
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