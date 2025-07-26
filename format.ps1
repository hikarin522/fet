#!/usr/bin/env pwsh

uncrustify -c uncrustify.cfg --no-backup (Get-ChildItem -Recurse include/fet/*.hpp).FullName
