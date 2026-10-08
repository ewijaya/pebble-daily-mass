# Releasing Daily Roman Missal

This is the procedure used by the project-local `missal-release` and
`missal-appstore` skills. [release-config.json](release-config.json) owns app
identity and paths; [RELEASE.md](RELEASE.md) records current readiness. This
procedure does not itself authorize tags, pushes, publication or visibility changes.

## Establish the destination and source

Inspect the working tree, selected version, origin, branch and remote releases.
Use `gh repo view --json nameWithOwner,visibility` and `gh release list` for GitHub;
read the authenticated store listing for drafts as well as published versions.
For a public release, fetch tags and confirm the exact source commit to tag.
A store-only release need not create a GitHub release. A preparation-only request
ends with the reviewable candidate and evidence.

The repository was private at setup. A GitHub Release there is restricted to
repository readers. Never change repository visibility automatically. If public
GitHub downloads are requested, resolve the public destination before publishing;
keep private source and the licensed content out of any new public repository.
The npm `private` flag is unrelated to GitHub visibility.

Review changes since the previous release tag; if no tag exists, review the
candidate scope itself. Version lives in root `package.json`. The independent
`tools/calendar/package-lock.json` pins calendar dependencies and does not need
a watch-version bump. No CI workflow existed when this procedure was added:
record local checks honestly rather than claiming green CI. Use CI if added later.

Do not clean someone else's work to satisfy a release preflight. Prepare notes
and inspect state while a tree is dirty; establish a committed source snapshot
before tagging. A request to maintain these skills does not authorize a commit.

## Build and validate

Read [README.md](../README.md) for the actual build commands. Both source files
are required: `data/source/*.epub` and `data/source/john-8-12-20.json`. They and
all generated Scripture databases remain ignored. Confirm the existing ignores
with `git check-ignore`; never print `.lock-waf*` contents (environment data).

For runtime/content changes, regenerate the appropriate resources and run:

```sh
python3 tests/test_reader.py
python3 tests/test_calendar.py
python3 tests/test_glance.py
python3 tests/test_planner.py
```

Use one clean final SDK build when a new candidate is needed. Preserve captures
and other useful evidence before cleaning. The SDK has previously missed
metadata-only PBW changes; archive the old ignored build output if necessary,
then check the metadata INSIDE the resulting PBW. Do not rebuild a frozen
candidate merely to upload it. Listing-only edits need no SDK build or reader tests.

Validate `appinfo.json` inside the ZIP against the configuration and version:
UUID, both names, `targetPlatforms == ["emery"]`, and watchapp type. Measure PBW
bytes, SHA-256, and uncompressed `emery/app_resources.pbpack` bytes. Resources
must fit the recorded PT2 firmware limit. Compare RAM and resource growth to the
latest measured candidate, without borrowing the other apps' tighter budgets.
Linker free heap is not measured runtime free heap.

The old SDK 256 KiB store warning did not block Missal's actual October 8 upload.
The store then reported a 4.4 MiB PBW ceiling; re-read current upload constraints
when relevant rather than freezing that service limit into a permanent rule.

Test flows affected by the change: local date, memorial/feast names, font
persistence, pagination and long reading boundaries. Calendar changes warrant
cycle/transfer regressions. Simulate dates on Emery only and restore real time;
CLI reconnects reset emulator time, so date tests need one connection. Do not
wipe the emulator or alter the physical watch clock to prepare a release.
Record physical readability, navigation, offline, midnight and battery evidence
separately; retain pending checks as pending. See [RELEASE-AUDIT.md](RELEASE-AUDIT.md).

## Freeze one candidate

Use a new `.release/VERSION/candidate-N/` directory; fail if it already exists.
Copy the tested PBW, release notes and any intended description/art changes there.
Do not populate it by rebuilding. Write `manifest.json` with:

- version, UUID, source commit and tag, selected destinations;
- PBW filename, byte count and SHA-256; resource bytes and measured RAM/heap;
- notes and description hashes, plus hashes/order of any assets to be changed;
- hashes/provenance of local source inputs, without publishing their contents;
- relevant test evidence and the installed PT2 digest/result;
- existing store app/release IDs and observed visibility;
- actual authorization reference/owner words, when already given;
- per-destination progress and verification results, initially pending.

Keep frozen inputs immutable; append status/evidence without silently replacing
bytes. A changed PBW needs a new candidate. Notes/listing changes need renewed
review of those changes, not an unnecessary re-test of identical app bytes.
If the user already tested precisely these bytes, reuse that evidence. Otherwise
install the frozen file explicitly with `pebble install --cloudpebble PATH`.
Phone-side failures have cleared after reopening the companion app; check the
reported result, reconcile state and make one informed retry rather than loop.

Explain the concrete candidate and destinations. Use session authorization when
it covers them. If publication scope is missing, ask once after preparation;
don't require the owner to type a hash when their approval clearly identifies
this candidate. Never invent physical approval or manufacture it with CLI flags.

## GitHub Releases

Check local/remote tags and existing release assets first. A matching remote
version/commit/PBW is already complete; a mismatch is not permission to overwrite.
After release authorization, push the approved source commit, then its annotated
tag. Set `MISSAL_REPO`, `MISSAL_VERSION`, `MISSAL_COMMIT`, `MISSAL_TAG` and
`MISSAL_CANDIDATE` from the reviewed configuration/manifest, never shell text
constructed from untrusted release notes.

For a missing tag and release, the sequence is:

```sh
git push origin "$MISSAL_COMMIT:refs/heads/main"
git tag -a "$MISSAL_TAG" "$MISSAL_COMMIT" -m "Daily Roman Missal $MISSAL_VERSION"
git push origin "refs/tags/$MISSAL_TAG"
gh release create "$MISSAL_TAG" "$MISSAL_CANDIDATE/daily-roman-missal.pbw" \
  --repo "$MISSAL_REPO" --verify-tag \
  --title "Daily Roman Missal $MISSAL_VERSION" \
  --notes-file "$MISSAL_CANDIDATE/notes.md"
```

Use `--draft` or `--prerelease` only for that requested mode. Never force a push,
replace an existing tag, or use upload `--clobber` to conceal a mismatch. If a
matching GitHub draft exists, reconcile it and publish that draft when authorized
instead of calling create again. Source archives attached automatically by GitHub
must not contain the ignored EPUB, supplement, credentials or generated text.

Download the named release's PBW with `gh release download TAG --repo REPO
--pattern daily-roman-missal.pbw --dir FRESH_DIRECTORY`. Compare its SHA-256 to
the frozen file; verify tag commit, title, notes, draft/prerelease state and
intended Latest behavior. Never verify a pre-existing local download instead.

## Pebble App Store

Operate on the configured existing listing. Current recorded drafts are 1.0.0
and 1.0.1; their identifiers are historical evidence, not proof of live state.
Read the dashboard record and match its ID and `app_uuid` to the configuration.
Keep the first baseline across retries. No registration or generated replacement
icons are needed. Source/website fields should not automatically advertise a
private repository. The known public lookup path is in the configuration.

The installed pebble-tool 5.0.40 `publish` command both rebuilds and hardcodes
`visible=true`/`isPublished=true`, ignoring its advertised unpublished default.
Do not use it for this workflow or blindly call its private uploader. The
previous scripts under ignored `artifacts/release-prep/` are evidence, not a
maintained generic release CLI. The sibling `release.py`/`upload_store.py`
helpers are NOT installed in Missal and must not be invoked against this repo.

Use the official dashboard operations observed on October 8, 2026; re-inspect
current frontend/API if the schema changes. Authentication uses the installed
Pebble Tool Python's `get_account(auth_provider="firebase")`, exchanging the
token at `https://developer.repebble.com/api/auth/firebase/session` in an in-memory
HTTP session. Never log the token or cookies. Disable redirects on authenticated
requests; credentials go only to the intended official auth/dashboard hosts.

| Operation | Dashboard route and payload |
| --- | --- |
| Read app | GET `/api/dashboard/apps/{app_id}`; response under `app` |
| Add a candidate | POST `/api/dashboard/apps/{app_id}/releases`; multipart `pbwFile`, `version`, `releaseNotes`, explicit `isPublished=false` |
| Promote existing draft | PATCH `/api/dashboard/apps/{app_id}/releases`; JSON `releaseId`, `isPublished: true` |
| Edit listing | PATCH `/api/dashboard/apps/{app_id}`; multipart intended fields, including `visibility` when its change is authorized |

Authenticated release responses use `is_published`; mutation payloads use
`isPublished`. Before a same-version upload, read existing releases and compare
the remote PBW digest. If the matching unpublished candidate already exists,
promote it when authorized. Different bytes for an occupied version need a new
version or a reviewed recovery decision; do not overwrite or delete the release.
If draft bytes cannot be verified, do not assume equivalence from version alone.

For first public availability of this existing hidden app, publish only the
approved release, then change visibility to `listed` as part of the authorized
public rollout. Leave earlier drafts unpublished. A published release in a
hidden listing is still not publicly available. Description-only changes preserve
visibility. Multipart PATCH semantics must be checked; preserve unrelated
metadata, companions and all artwork/order through before/after read-backs.

## Verify and recover

For an unpublished hidden candidate, verify dashboard flags and uploaded metadata;
an anonymous 404 is expected. For public publication, verify:

1. Dashboard: app identity, selected release, notes, main/Emery description and
   unchanged fields/assets, except the explicitly authorized changes.
2. General and `?hardware=emery` UUID catalogs: matching app ID, version, release
   notes and platform (platform entries are objects).
3. Canonical `store_listing_url`: version, description, changelog and screenshots.
4. Downloaded public `pbw_file`: hash matches the frozen PBW. Send no dashboard
   credentials to a returned asset URL. If signed links expire, refresh read-only.

On lost upload responses, discover remote state before any retry or UI fallback.
If GitHub completes and the store fails, keep GitHub intact and record partial
completion. Retry only the missing step after reconciliation. Use at most three
read-only propagation attempts with roughly 20-second intervals and concise
updates. Report unresolved surfaces rather than rebuilding, republishing or
silently replacing the baseline. A fresh Emery catalog does not prove the phone
client refreshed; observe My Apps before making that claim.

Update README availability and release status only for verified destinations.
Record the final source commit/tag, artifact digest, metrics, observed tests,
URLs, pending checks and final Git status. Do not expose private input paths or
authentication details in public release notes.
