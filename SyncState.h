#ifndef SYNCSTATE_H
#define SYNCSTATE_H

/*
 * SyncState.h
 * Manages parameter synchronization between ESP32, Amphonix 2 amp, and Sonicake app.
 *
 * Key features:
 *   - Tracks "dirty" parameters (locally edited)
 *   - Guards against reverting edits with stale dumps
 *   - Prevents sync loops between app ↔ amp
 *   - Maintains canonical values with edit timestamps
 */

#include <Arduino.h>
#include <string.h>

class SyncState {
public:
    SyncState();

    // ── Initialization ────────────────────────────────────────────────
    void begin();

    // ── Parameter accessors (all 0–100) ───────────────────────────────
    uint8_t paramPV;
    uint8_t paramAmpVol;
    uint8_t paramAmpGain;
    uint8_t paramAmpTreb;
    uint8_t paramAmpMid;
    uint8_t paramAmpBass;
    uint8_t paramDrvVol;
    uint8_t paramDrvGain;
    uint8_t paramDrvTone;

    // ── Dirty flag management ─────────────────────────────────────────
    // Mark a specific encoder's parameter as "locally edited"
    void markParamDirty(uint8_t encIdx, uint8_t paramIdx);

    // Clear all dirty flags (call after successful sync to amp)
    void clearDirtyFlags();

    // Check if a parameter was edited recently (within guard window)
    bool isParamDirty(uint8_t encIdx, uint8_t paramIdx) const;

    // ── Update from amp dump ──────────────────────────────────────────
    // Called when a complete dump arrives from the amp.
    // Returns true if any values actually changed.
    bool updateFromAmp(uint8_t presetIdx,
                       uint8_t _pv,
                       uint8_t _ampVol,  uint8_t _ampGain, uint8_t _ampTreb,
                       uint8_t _ampMid,  uint8_t _ampBass,
                       uint8_t _drvVol,  uint8_t _drvGain, uint8_t _drvTone);

    // ── App write detection ───────────────────────────────────────────
    // Call when the app sends data; triggers a short guard window
    void onAppWrite();

    // ── Update cycle (call from loop) ────────────────────────���────────
    // Manages timeout logic for dirty flags and app-write guards
    void updateDirtyFlags();

    // ── Query current preset ──────────────────────────────────────────
    uint8_t getCurrentPreset() const { return currentPresetIdx; }
    void setCurrentPreset(uint8_t idx) { currentPresetIdx = idx; }

private:
    uint8_t currentPresetIdx;

    // Dirty flag tracking: per-encoder, per-parameter
    // dirty[encIdx][paramIdx] = timestamp (0 = not dirty)
    static const uint8_t MAX_ENC = 4;
    static const uint8_t MAX_PARAM_PER_ENC = 3;
    uint32_t dirtyTime[MAX_ENC][MAX_PARAM_PER_ENC];

    // Guard window after app writes data
    uint32_t appWriteGuardMs;
    bool     appWriteGuardActive;

    static const uint32_t DIRTY_GUARD_MS = 2000;    // Parameter "dirty" for 2 s
    static const uint32_t APP_WRITE_GUARD_MS = 600; // Ignore dumps 600 ms after app write
};

#endif // SYNCSTATE_H
