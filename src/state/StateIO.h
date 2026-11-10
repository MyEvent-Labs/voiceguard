#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace voiceguard
{

/** Root type of a serialised VoiceGuard state tree. */
inline constexpr const char* kStateRootType = "VoiceGuardState";

/** Current on-disk/state schema version. */
inline constexpr int kStateVersion = 1;

/** Serialise a plugin state tree to XML text, stamping the current version. */
juce::String stateToXml (const juce::ValueTree& state);

/** Parse XML text into a state tree.

    Fails safely (returns an invalid tree and a failed Result) on empty input,
    malformed XML, a wrong root type, or a schema version newer than this
    build. Older schemas are migrated forward in place. */
juce::ValueTree stateFromXml (const juce::String& text, juce::Result& result);

/** Apply forward migrations in place. Unknown/older versions are upgraded to
    kStateVersion. Callers must still validate the resulting tree. */
void migrateState (juce::ValueTree& state, int fromVersion);

} // namespace voiceguard
