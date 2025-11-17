# Cpp Sandbox

A personal C++ playground where I partice architecture ideas, concurrency, design patterns, networking, and various low-level techniques. The project contains a variety of small, unrelated tests that serve as isolated environments for trying out new concepts.
All of the source code is located in the */code* folder.

## Repository Structure
- code/tests :
  compiles into executables. Serves as tests and implementations of the libraries below.

- code/shared :
  static library with various unsorted classes and helpers that get included into *every* test. 

- code/dataflow :
  static library that provides functionality for building node based processing graphs on top of a message bus. It also supports IPC between said nodes, currently implementing a single Win-Sock (WSA) backend. Is included only in tests that use it. Multithreaded.

- code/wsanet :
  static library that provides a minimal client-server functionality using Window Sockets. Is included in the *dataflow* library and some tests that use it.

The entire repository is a work in progress.
