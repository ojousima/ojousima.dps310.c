# ojousima.dps310.c
MIT-licensed portable driver for Infineon DPS310 pressure and temperature sensor

# Running unit tests
## Ceedling
Unit tests are implemented with Ceedling. Tun the tests with
`ceedling test:all`

### Gcov
Ceedling can also generate Gcov reports with `ceedling gcov:all utils:gcov`.
The report can be foun under _build/artifacts/gcov_.
