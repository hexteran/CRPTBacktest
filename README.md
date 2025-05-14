This small backtester is a part of my CRPT pet project. Its goal is to create a simple low-latency trading environment which would contain all necessary components to build a quantitative trading system.

<h2>What this repo contains</h2>
  This repo consists of three main parts:
  
- **Simulation Core**: a tick-size, event-based simulation engine.
- **Examples**: some python and C++ samples to start with.
- **Python API**: python interface for fast prototyping.

<h2>Requirements</h2>

  Before building make sure that you have these installed:
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
bash build.sh [options]
```

Build it with one of the options:
  - <code>-t</code>, <code>--tests</code> Build tests (requires gtest installed)
  
  - <code>-e</code>, <code>--examples</code> Build examples
  
  - <code>-p</code>, <code>--pystrategy</code> Build Python strategy module

For example:
````
bash build.sh --pystrategy --tests
````

After the build completes, you’ll find the <code>.so</code> library and a copy of <code>PyStrategy.py</code> in the build/python directory.
Be sure to include that directory (or any other that contains the aforementioned files) in your <code>PYTHONPATH</code>.

<h2>Python API</h2>
This backtester provides a Python interface based on pybind11; you can find it in examples/python.
To run it correctly, build the solution with the 
<b>--all</b> or <b>--pystrategy</b> flag.

Check out [wiki page](https://github.com/hexteran/CRPTBacktest/wiki/PyStrategy) to learn more about PyStrategy.

  


