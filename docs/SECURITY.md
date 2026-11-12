# Security and privacy

## Local by design

VoiceGuard processes audio entirely on the local machine. There is no account,
no network access, no telemetry and no transcription. Microphone audio never
leaves the computer. All targets compile with:

```
JUCE_USE_CURL=0
JUCE_WEB_BROWSER=0
JUCE_REPORT_APP_USAGE=0
```

`src/` contains no sockets, HTTP clients, `system()`/`popen`, or file IO on the
audio path.

## Trust boundaries

For a local plugin the untrusted inputs are:

- **Audio buffers from the host.** Treated as hostile: the engine sanitises every
  sample (NaN/Inf to 0), clamps coefficients and parameters, size-checks buffers,
  and bounds the output with a final safety stage. A host block larger than the
  prepared size is clamped rather than written out of bounds.
- **Serialised state blobs.** `setStateInformation` rejects blobs over 1 MiB,
  malformed XML, a wrong root type, and newer schema versions, falling back to the
  current safe state.

There is no authentication, session, database, crypto or server surface to
protect.

## Residual risks

- **Crash dumps.** Normal operation keeps audio local, but an OS crash reporter
  or third-party minidump tool could capture audio buffers in memory. Do not
  enable dump upload for show-critical use.
- **Signing.** Release artifacts are unsigned (macOS ad-hoc only); verify the
  published SHA-256 checksums.

## Reporting

Report a security concern privately through the repository's **Security
advisories** on GitHub (Security → Advisories → Report a vulnerability) rather
than a public issue.
