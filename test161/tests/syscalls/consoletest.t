---
name: "Console Test"
description: >
  Make sure that the console works and the kernel menu waits appropriately.
tags: [console]
depends: [boot]
sys161:
  ram: 2048K
---
p /testbin/consoletest
