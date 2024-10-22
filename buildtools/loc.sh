#!/bin/bash

# Count lines of code in morph
find . \( -path './*.[h]' -or -path './*.hpp' -or -path './*.cpp' \) \
     -or \( -path './morph/GL3' -or -path './build' -or -path './include' -or -path './morph/healpix' \) -prune \
     | xargs wc -l
