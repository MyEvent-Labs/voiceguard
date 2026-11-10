#include "../TestRegistry.h"

#include "state/StateIO.h"

using namespace voiceguard;

namespace
{
juce::ValueTree makeState()
{
    juce::ValueTree state (kStateRootType);
    state.setProperty ("cleanAmount", 0.62, nullptr);
    state.setProperty ("levelTargetDb", -17.5, nullptr);

    juce::ValueTree advanced ("Advanced");
    advanced.setProperty ("dynRatio", 2.5, nullptr);
    state.appendChild (advanced, nullptr);

    return state;
}
} // namespace

VG_TEST (stateRoundTrip)
{
    const auto original = makeState();
    const auto xml = stateToXml (original);
    VG_CHECK (xml.isNotEmpty());

    juce::Result result = juce::Result::fail ("unset");
    const auto restored = stateFromXml (xml, result);

    VG_CHECK (result.wasOk());
    VG_CHECK (restored.isValid());
    VG_CHECK (restored.hasType (kStateRootType));
    VG_CHECK_CLOSE ((double) restored.getProperty ("cleanAmount"), 0.62, 1.0e-9);
    VG_CHECK_CLOSE ((double) restored.getProperty ("levelTargetDb"), -17.5, 1.0e-9);
    VG_CHECK ((int) restored.getProperty ("version") == kStateVersion);
    VG_CHECK (restored.getChildWithName ("Advanced").isValid());
}

VG_TEST (stateRejectsCorruptBlob)
{
    juce::Result result = juce::Result::ok();
    const auto tree = stateFromXml ("this is not xml <<<", result);
    VG_CHECK (! tree.isValid());
    VG_CHECK (result.failed());

    juce::Result emptyResult = juce::Result::ok();
    const auto empty = stateFromXml ("", emptyResult);
    VG_CHECK (! empty.isValid());
    VG_CHECK (emptyResult.failed());
}

VG_TEST (stateRejectsWrongRoot)
{
    juce::XmlElement xml ("SomethingElse");
    juce::Result result = juce::Result::ok();
    const auto tree = stateFromXml (xml.toString(), result);
    VG_CHECK (! tree.isValid());
    VG_CHECK (result.failed());
}

VG_TEST (stateRejectsFutureVersion)
{
    juce::ValueTree future (kStateRootType);
    future.setProperty ("version", kStateVersion + 99, nullptr);

    juce::Result result = juce::Result::ok();
    const auto tree = stateFromXml (future.createXml()->toString(), result);
    VG_CHECK (! tree.isValid());
    VG_CHECK (result.failed());
}

VG_TEST (stateMigratesUnversioned)
{
    juce::ValueTree old (kStateRootType);
    old.setProperty ("cleanAmount", 0.4, nullptr); // no version property = 0

    juce::Result result = juce::Result::ok();
    const auto migrated = stateFromXml (old.createXml()->toString(), result);
    VG_CHECK (result.wasOk());
    VG_CHECK (migrated.isValid());
    VG_CHECK ((int) migrated.getProperty ("version") == kStateVersion);
    VG_CHECK_CLOSE ((double) migrated.getProperty ("cleanAmount"), 0.4, 1.0e-9);
}
