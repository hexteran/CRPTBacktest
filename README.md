This small backtester is a part of my CRPT pet project. Its goal is to create a simple low-latency trading environment which would contain all necessary components to build a quantitative trading system.

<h2>What this repo contains</h2>
  This repo consists of three main parts:
  
- Simulation Core
- Examples
- Python API

<h2>Requirements</h2>

  Before building make sure that you have these two installed:
  <code>CMake 3.5.0+</code>
  <code>Clang 18.0+</code>
  <code>GTest(optional)</code>

<h2>Build</h2>
Clone the repo


  ````bash
git clone https://github.com/hexteran/CRPTBacktest &&
cd CRPTBacktest
  ````

```
./build.sh [options]
```

Build it with one of the options:
  - <code>-t</code>, <code>--tests</code> Build tests (requires gtest installed)
  
  - <code>-e</code>, <code>--examples</code> Build examples
  
  - <code>-p</code>, <code>--pystrategy</code> Build Python strategy module
  
  - <code>-a</code>, <code>--all</code> Build everything (tests, examples, pystrategy)
  
  - <code>-h</code>, <code>--help</code> Show this help message and exit

For example:
````
./build.sh --pystrategy --tests
````
  


