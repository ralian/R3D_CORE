# CONTRIBUTING

Please follow the relevant checklists and guidelines when contributing to R3D_CORE (or just Core for short.) This is not the code of conduct. Please also read and follow that when working with our community.

## Opening an Issue

When you _should_ open an issue:

- You have noticed a script or config problem with core which leads to script errors, client or server crashes, degradation of performance, or any other undesirable behavior.
- You have a mod which depends on R3D_CORE, and some part of core is not working as expected / could be redesigned to better accommodate your and other mods.
- You have a reasonable feature request for our team, or an idea to improve an existing feature.

When you _should not_ open an issue:

- You have noticed a problem with one of the project redline mods which depends on core, but is not a part of R3D_CORE!
  - If this is the case, please raise the issue with us on Discord. We will close issues opened in this repo for other mods.
- You swear that R3D_CORE is crashing your server/game, but you don't have any logs/repro steps/evidence to back it up.
- Bugs in the base game should first go to the [Arma Reforger](https://feedback.bistudio.com/project/view/66/) or [Arma Reforger Modding](https://feedback.bistudio.com/project/view/68/) issue pages.
  - In some cases where these problems are tightly coupled with a specific bug in game, it may still be appropriate to open an issue for us to tackle once the base game is patched. Ask us first.

Before you open an issue:

1. Please search the existing issues before creating a new, possibly duplicate issue!
2. If your issue is for a new feature, you should first read below "What belongs in R3D_CORE." If it does belong in core, you might also want to ask us if it's being worked on. If it is not being worked on and is a reasonable addition, open an issue.
3. If your issue is a bug, _please attach all relevant logfiles and reproduction steps._ I cannot emphasize this enough. Issues which do not include this will be closed. Follow these guidelines:
   - Preferably, reproduce the bug with only R3D_CORE loaded to rule out other mod issues. If the issue only arises with another mod loaded, please only load that other mod.
     - We understand some issues only show up on heavily loaded servers, sometimes with many mods loaded. We will still process issues that have more mods loaded, but loading only core will make the process far easier.
   - Make sure to include all 4 generated logfiles from your profile dir even if you don't think they are important: the `console.log`, `crash.log`, `error.log`, `script.log`.
   - If the bug is network related, you may want to attach logs from the dedicated server as well as the client.
   - Some kind of crashes do not show up in the log! If this is the case, still include the logs to be sure - but give us a description of what you were doing on the client/server when the crash happened. Fortunately these cases are rare.
4. If you read all the prior steps, you can probably submit now! If you don't hear back in 24h, asking us about it on Discord is fine.

## What belongs in R3D_CORE?

R3D_CORE started as a way to consolidate some of our assets and scripts used in multiple vehicle mods, to save download space and bandwidth. This is especially important on Console platforms, since there is a hard space limit for mods of 20GB.



Consequently, there are a few constraints on core:

- It must be reasonable in size, above all else.
- It must be very stable, and affect performance very little.
- Scripts added must be broadly applicable.
- Assets added must be small and broadly useful.

In open sourcing the project on github, we also aim to keep all of the files within Github's constraints on a free account. This means no individual file larger than 100MB, currently.



## Opening a PR

#todo