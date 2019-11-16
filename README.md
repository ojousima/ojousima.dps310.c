# ojousima.dps310.c
[![Build Status](https://travis-ci.org/ojousima/ojousima.dps310.c.svg?branch=master)](https://travis-ci.org/ojousima/ojousima.dps310.c)

MIT-licensed portable driver for Infineon DPS310 pressure and temperature sensor

# Running unit tests
## Ceedling
Unit tests are implemented with Ceedling. Run the tests with
`ceedling test:all`

### Gcov
Ceedling can also generate Gcov reports with `ceedling gcov:all utils:gcov`.
The report can be foun under _build/artifacts/gcov_.

# Code style
Code shall be formatted with Artistic Style using .astylerc in project root as the
ruleset.
```
astyle --project=.astylerc --recursive "./src/*.c"
astyle --project=.astylerc --recursive "./src/*.h"
```

# Static code analysis
Run `make` to run PVS Studio on project. Report can be found at _pvs.tasklist_.

# Documentation
Run `make` to generate Doxygen docs with git version information. 

# Continuous integration pipeline
The project is built on Travis and build status is reflected on a badge on top of the 
header.