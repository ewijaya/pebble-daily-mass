---
name: missal-appstore
description: Inspect, update and verify Daily Roman Missal's existing Pebble App Store listing, hidden drafts, description, icons, screenshots and Emery catalog status. Use for Missal store work; versioned publication also uses missal-release.
---

# Daily Roman Missal App Store

Read [release-config.json](../../../docs/release-config.json),
[store operations](../../../docs/RELEASING.md#pebble-app-store), and
[current release status](../../../docs/RELEASE.md). For versioned publication,
use [missal-release](../missal-release/SKILL.md).

## Identity and scope

The app already has a registered listing. Match BOTH its configured store ID
and PBW UUID in the authenticated response. Its launcher/store name is
**Roman Missal**; the full brand **Daily Roman Missal** belongs in the description.
Keep both PBW names compact. Do not borrow Popeye/Orationes IDs, artwork or tags.

The recorded starting state is hidden with unpublished releases. Read live
state before any change; an anonymous 404 is expected for a hidden listing and
is not a reason to register another one. Do not publish a draft or change
visibility during a listing-only request. Maintaining this skill changes no
external state.

## Prepare the intended change

- Use `release/store/listing.md`: upload only its Description section as the
  description and Release notes section as release notes, not the whole file.
- Use the approved Chi-Rho icons and native Emery screenshots. Preserve their
  existing order and files unless replacement/reordering is in scope. When
  preparing a new set, put the main menu first. Don't generate replacement icons.
- State the actual shipped scope: offline readings, General Roman Calendar,
  supported years/platform, selected reading forms and omitted vigils. Retain
  source attribution; don't claim a complete or officially approved Missal.
- Snapshot the first remote baseline: title, description (including platform
  asset descriptions), category, visibility, website/source, companions, icons,
  banners, screenshot order and every prior release. Keep it across retries.
- Reuse existing session authorization for the requested fields. A routine
  version upload does not imply permission to replace art or alter visibility.

## Apply and verify

Prefer the inspected official dashboard API for supported operations, using
existing Pebble credentials. Use browser UI for login or a specific operation
whose API is unavailable; explain the gap before switching. Never print tokens,
save session cookies, or send credentials to asset/CDN URLs.

Check same-version releases before uploading. An identical unpublished candidate
can be promoted after publication is authorized; different bytes require a new
version or an explicit recovery decision. Keep older drafts unpublished unless
independently in scope. Never call Dashboard New for this existing app.

After a listing PATCH, read back the main description and Emery asset description,
and compare every unrelated field with the saved baseline. After publication,
verify the dashboard, general and Emery public catalogs, canonical public page,
release notes and downloaded PBW hash. For a hidden draft, verify hidden state,
unpublished status and anonymous 404 instead of treating invisibility as failure.

Poll read-only at most three times, then report any stale surface. If only mobile
My Apps is stale, suggest reopening it; do not delete/re-add the app merely to
make a verification claim. Upload success alone is not publication verification.
