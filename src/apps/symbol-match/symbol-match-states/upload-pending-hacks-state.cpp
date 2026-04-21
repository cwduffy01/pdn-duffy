#include "apps/symbol-match/symbol-match-states.hpp"
#include "apps/hacking/hacked-players-manager.hpp"
#include "device/drivers/logger.hpp"
#include "fdn-constants.hpp"
#include "utils/display-utils.hpp"
#include <string>

#define TAG "SYMBOL_UPLOAD_PENDING"

SymbolMatchUploadPendingHacksState::SymbolMatchUploadPendingHacksState(HackedPlayersManager* hackedPlayersManager)
    : State(SymbolMatchStateId::SYMBOL_MATCH_UPLOAD_PENDING) {
    this->hackedPlayersManager = hackedPlayersManager;
}

SymbolMatchUploadPendingHacksState::~SymbolMatchUploadPendingHacksState() {
    hackedPlayersManager = nullptr;
}

void SymbolMatchUploadPendingHacksState::onStateMounted(Device* PDN) {
    contentReady = false;
    completedCount = 0;

    auto pending = hackedPlayersManager->getPendingUploads();
    pendingCount = static_cast<int>(pending.size());

    LOG_I(TAG, "Mounted - %d pending upload(s)", pendingCount);

    for (const auto& playerId : pending) {
        std::string payload =
            "{\"playerId\":\"" + playerId +
            "\",\"boxId\":" + std::to_string(static_cast<int>(FDN_BOX_ID)) +
            ",\"hacked\":true}";

        HttpRequest req(
            "/api/boxes",
            "POST",
            payload,
            [this, playerId](const std::string&) {
                LOG_I(TAG, "Upload succeeded for player %s", playerId.c_str());
                hackedPlayersManager->playerHackUploaded(playerId);
                completedCount++;
            },
            [this, playerId](const WirelessErrorInfo& err) {
                LOG_E(TAG, "Upload failed for player %s: %s", playerId.c_str(), err.message.c_str());
                completedCount++;
            }
        );

        PDN->getHttpClient()->queueRequest(req);
    }

    fallbackTimer.setTimer(FALLBACK_TIMEOUT_MS);
    glyphTimer.setTimer(GLYPH_LOADING_DURATION_MS);
    showLoadingGlyphs(PDN);
}

void SymbolMatchUploadPendingHacksState::onStateLoop(Device* PDN) {
    if (isInGlyphLoadingPhase(PDN, glyphTimer)) return;

    if (!contentReady) {
        PDN->getDisplay()
            ->invalidateScreen()
            ->drawText("SYNCING", 30, 32)
            ->render();
        contentReady = true;
    }
}

void SymbolMatchUploadPendingHacksState::onStateDismounted(Device* PDN) {
    (void)PDN;
    LOG_I(TAG, "Dismounted - %d/%d uploads completed", completedCount, pendingCount);
    glyphTimer.invalidate();
    fallbackTimer.invalidate();
    contentReady = false;
    pendingCount = 0;
    completedCount = 0;
}

bool SymbolMatchUploadPendingHacksState::transitionToIdle() {
    if (pendingCount == 0) return true;
    return completedCount >= pendingCount || fallbackTimer.expired();
}
