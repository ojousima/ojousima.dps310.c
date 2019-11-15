# ojousima.dps310.c
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
astyle - project=.astylerc - recursive "./tasks/*.c"
astyle - project=.astylerc - recursive "./tasks/*.h"
```
