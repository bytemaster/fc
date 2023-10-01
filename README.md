# Title
FC -- Fast-Compiling C++ Library

# Project Description
FC provides a set of utility libraries useful for the development of asynchronous libraries. Some of the highlights include:

 - Cooperative Multi-Tasking Library with support for Futures, mutexes, signals.
 - Wrapper on Boost ASIO for handling async opperations cooperatively with easy to code synchronous style.
 - Reflection for C++ allowing automatic serialization in Json & Binary of C++ Structs 
 - Automatic generation of client / server stubs for reflected interfaces with support for JSON-RPC
 - Cryptographic Primitives for a variaty of hashes and encryption algorithms
 - Logging Infrastructure 
 - Wraps many Boost APIs, such as boost::filesystem, boost::thread, and boost::exception to acceleate compiles
 - Support for unofficial Boost.Process library.

# How to Install/Run Project
## Prerequisites:
Compiler: Ensure you have a modern C++ compiler (such as g++ or clang++).

CMake: FC likely uses a CMake build system. Ensure you have CMake installed.

Boost Libraries: Since FC wraps and utilizes various Boost libraries, you'll need to have Boost installed.

## Step-by-step Installation:
1. Clone the Repository. you'll need [Git](https://git-scm.com)
```bash
# Clone this repository
$ git clone git@github.com:bytemaster/fc.git

# Go into the repository
$ cd fc
```
2. Initialize and Update Submodules
```bash
$ git submodule init
$ git submodule update
```
3. Configure with CMake
```bash
$ cmake .
```
4. Compile the Library
```bash
$ make
```
5. Install the Library System-wide

```bash
$ sudo make install
```
6. Testing -- run the test suite to ensure everything was set up correctly
```bash
$ make test
```
# MIT License 
Copyright (c) 2010 Respective Authors

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.