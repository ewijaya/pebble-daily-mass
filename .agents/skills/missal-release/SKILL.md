---
name: missal-release
description: Prepare, freeze, publish and verify a Daily Roman Missal release for GitHub Releases, the Pebble App Store, or both. Use for Missal release preparation, shipping or release verification; not ordinary builds. Reading or maintaining this skill does not authorize publication.
---

# Daily Roman Missal release

Read [the release procedure](../../../docs/RELEASING.md),
[identity configuration](../../../docs/release-config.json), and
[current release status](../../../docs/RELEASE.md). Run from the repository root.
For an App Store destination, also read [missal-appstore](../missal-appstore/SKILL.md).

## Scope and preparation

Establish version, destinations and whether the request is preparation, a draft,
or publication. Reuse authorization already present in the session; don't ask
again for actions it covers. A favorable watch test is evidence of quality,
not automatically a request to publish to both services. Finish the candidate,
notes and listing before asking for any missing publication decision.

1. Inspect the actual diff, branch, remotes, tags and destination state. Use the
   configured Missal UUID and store ID, never values from the sibling projects.
2. Preserve unrelated work. Drafting notes and inspecting an existing candidate
   do not require a clean tree. A public release needs a recorded source commit;
   commit only authorized changes, without stashing or discarding others' work.
3. Choose version from shipped changes, explaining an inferred version before
   editing. Read existing versions first: a hidden store candidate may already
   occupy that version. The calendar npm lockfile is a build dependency lock,
   not the watch app's version file.
4. Run the applicable checks in RELEASING.md. Keep the original EPUB and the
   owner-supplied Gospel supplement local. Never rewrite reading text merely
   to prepare a release, or reuse another project's size budgets or QA suite.
5. Freeze the tested PBW, notes, intended listing changes, hashes and evidence.
   Install that exact file on PT2. Reuse earlier installation/test evidence
   only when the digest matches; report unobserved battery/physical checks.

## Publication

Use only frozen bytes. Do not rebuild as part of publication or upload from
mutable `build/`. If the existing session authorizes this candidate and these
destinations, proceed. Otherwise present its version, digest, notes, listing,
destinations and remaining checks in one concrete review request.

For both destinations, publish and verify GitHub first, then the store. A
private GitHub repository produces a restricted release: do not change repository
visibility or claim a public download without the owner's chosen public
hosting arrangement. An App Store-only request does not require a GitHub release.

Do not run top-level `pebble publish`: the inspected installed version rebuilds
and hardcodes public flags. Follow the documented GitHub and dashboard operations.
Promote the matching existing Missal draft when authorized; do not register a
second app or re-upload the same version blindly.

## Recovery and reporting

After a timeout, read the remote state before retrying. Matching tag, version
and downloaded bytes mean that step is complete; conflicting bytes or tag
commits need reconciliation, not overwrite or deletion. Persist partial progress.
Bound propagation verification to three read-only attempts with short intervals,
then report pending surfaces. Never turn elapsed time into a passed check.

Update availability claims only for verified destinations. Report version,
source commit/tag, PBW digest, test/device evidence, GitHub and store URLs/status,
remaining checks and Git state. A phone cache is current only when observed.

This workflow adapts the sibling Popeye G&W and Orationes release skills;
Missal's own configuration and current API behavior take precedence.
