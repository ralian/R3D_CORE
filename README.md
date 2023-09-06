# Project Redline: Core

Project Redline is the most comprehensive collection of modded vehicles for Bohemia Interactive's pilot Arma project, Arma Reforger. The Core mod is used by all the other mods as a dependency, and is also a dependency of many other community mods.

## Q&A



Q: Why are you making R3D_CORE (or just "core") open source?

> We want the community to be able to contribute features to Core directly, and have a convenient way to open issues. Although we try and check every bit of our work for performance, stability, and compatibility issues, on occasion a bug (script crash, inefficient code, floating building, etc…) has slipped by. In the past users would have to contact us directly, or submit a patch mod on top of R3D_CORE. This is not optimal, and we'd like to do better as Core grows.

Q: If the source for Core was already public and visible (through Workbench), why wasn't Core open source before?

> Practical reasons. Actually, R3D_CORE didn't exist when we started project redline and released the UH-60, our first real mod. The issue with storing an entire mod on a git server is mainly size - textures and source models contribute greatly to consumed space, and premium github/gitlab accounts are expensive. There are ways to work around this, but hosting an art drive or self hosting an entire git server just isn't a practical use of our team's time.
> 
>
> Originally we used gitlab because its pricing scheme is _slightly_ more generous than Github. However, once our art repos grew in size, we had to look for other alternatives, and settled on AWS CodeCommit for a git host due to its very practical pricing scheme. Unfortunately, CodeCommit is not as user friendly and its complexity doesn't make bringing new users in to work on it very easy. So, all our art and mod repos will remain closed source - except for Core, which is now open source on Github.

Q: What if I want to help work on one of your projects that isn't in Core?

> Hail us on Discord.

Q: I have a fix or new feature ready but don't know how to use Git. What do I do?

> Again, contact us. If you are willing to help with a fix, feature, item, or art, we can help you with git. Just start by making a Github account.

