#include "state/StateIO.h"

namespace voiceguard
{

juce::String stateToXml (const juce::ValueTree& state)
{
    auto copy = state.createCopy();
    copy.setProperty ("version", kStateVersion, nullptr);

    if (auto xml = copy.createXml())
        return xml->toString();

    return {};
}

juce::ValueTree stateFromXml (const juce::String& text, juce::Result& result)
{
    result = juce::Result::ok();

    if (text.isEmpty())
    {
        result = juce::Result::fail ("empty state");
        return {};
    }

    auto xml = juce::XmlDocument::parse (text);

    if (xml == nullptr)
    {
        result = juce::Result::fail ("malformed state XML");
        return {};
    }

    auto tree = juce::ValueTree::fromXml (*xml);

    if (! tree.isValid() || ! tree.hasType (kStateRootType))
    {
        result = juce::Result::fail ("unexpected state root");
        return {};
    }

    const int version = (int) tree.getProperty ("version", 0);

    if (version > kStateVersion)
    {
        result = juce::Result::fail ("state was written by a newer VoiceGuard version");
        return {};
    }

    if (version < kStateVersion)
        migrateState (tree, version);

    return tree;
}

void migrateState (juce::ValueTree& state, int fromVersion)
{
    // Version 0 = pre-versioning; nothing structural to repair, just stamp.
    if (fromVersion < 1)
        state.setProperty ("version", 1, nullptr);

    state.setProperty ("version", kStateVersion, nullptr);
}

} // namespace voiceguard
