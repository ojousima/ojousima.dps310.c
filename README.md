# ojousima.dps310.c
MIT-licensed portable driver for Infineon DPS310 pressure and temperature sensor
[![Build Status](https://travis-ci.org/ojousima/ojousima.dps310.c.svg?branch=master)](https://travis-ci.org/ojousima/ojousima.dps310.c)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=ojousima.dps310.c&metric=alert_status)](https://sonarcloud.io/dashboard?id=ojousima.dps310.c)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=ojousima.dps310.c&metric=bugs)](https://sonarcloud.io/dashboard?id=ojousima.dps310.c)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=ojousima.dps310.c&metric=code_smells)](https://sonarcloud.io/dashboard?id=ojousima.dps310.c)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=ojousima.dps310.c&metric=coverage)](https://sonarcloud.io/dashboard?id=ojousima.dps310.c)
[![Duplicated Lines (%)](https://sonarcloud.io/api/project_badges/measure?project=ojousima.dps310.c&metric=duplicated_lines_density)](https://sonarcloud.io/dashboard?id=ojousima.dps310.c)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=ojousima.dps310.c&metric=ncloc)](https://sonarcloud.io/dashboard?id=ojousima.dps310.c)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=ojousima.dps310.c&metric=sqale_rating)](https://sonarcloud.io/dashboard?id=ojousima.dps310.c)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=ojousima.dps310.c&metric=reliability_rating)](https://sonarcloud.io/dashboard?id=ojousima.dps310.c)
[![Technical Debt](https://sonarcloud.io/api/project_badges/measure?project=ojousima.dps310.c&metric=sqale_index)](https://sonarcloud.io/dashboard?id=ojousima.dps310.c)

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
astyle --project=.astylerc --recursive "./src/*.c" "./src/*.h"
```

# Static code analysis
Run `make` to run PVS Studio on project. Report can be found at _doxygen/pvs/fullhtml/index.html_.
[Here](./fullhtml/index.html) is a link to documentation through Doxygen-generated documentation.

# Documentation
Run `make` to generate Doxygen docs with git version information. 

# Continuous integration pipeline
The project is built on Travis and build status is reflected on a badge on top of the 
header. SonarCloud generates additional analytics and badges for the project.

# Using the project as a template. 
If you wish to use this project as a template for your own project, checkout tag v0.1.0
and change the project name and file names and setup the continuous integration pipeline
for the project.