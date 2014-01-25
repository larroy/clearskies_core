# CONTRIBUTING

We welcome new contributors


Fork the project [on GitHub](https://github.com/larroy/clearskies_core) and check out
your copy.

```
$ git clone https://github.com/<your user>/clearskies_core.git
$ cd clearskies_core 
$ git remote add upstream https://github.com/larroy/clearskies_core.git
```

master is our development branch, we branch for features with the prefix feature_<feature_name>

Bug fixes can go directly on the master branch, more complex features should go in a feature branch

You can create a feature branch:

```
$ git checkout -b my-feature-branch -t origin/master
```

Write descriptive commit messages and organize commits in logical changes, best to sparate one
commit that changes two unrelated things into two for example.


### TEST

A [TDD]: http://en.wikipedia.org/wiki/Test-driven_development approach to writing code is
recommended, there's unit tests in the test directories. Code that's testable is often modular and
with better design than otherwise.

Run the existing tests after you make changes. And provide test for new implemented features.

### CODING STYLE:

Trivia:

* 4 spaces for indentation
* Member variable names prefixed with m_ inside a class and underscores.
 This improves legibility as class types are visible different than instances or variables.
* Variables with underscores: int count_elements = 0;
* Class names in cammel case instances with underscores: FooMachine foo_machine();
* Private functions inside a cpp file inside anonymous namespace
* namespaces instead of classes with all static methods
* prefer a function instead of a class when possible
* avoid deep class hierarchies


### MODERN C++ CODING BEST PRACTICE:

C++ is a complex language, but there are good practices to avoid shooting yourself in the foot too
often:

* Order of **function definition** in the cpp file should be the **same is in the header** (function
declaration)
* Always **initialize variables**, both local variables and member variables in the constructor
 initialization list, including non-pods, as this practice prevents bugs in the long run. 
* Initialize variables in the constructor initialization list in the **same order** that they are
declared in the class.
* Use the correct, **portable types**, ie. size_t for loops. Use range based for loops when possible.
* Use **RAII** idiom, the only exception is when interacting with C code. This means use smart pointers
to handle memory allocations so resources are not leaked when exceptions are handled, if you use the
keywords "new" and "delete" you are doing something wrong. Notable exceptions are inside a constructor
or a destructor, or when interacting with C code.

This is OK:

    auto x = make_unique<Class>();


This is NOT OK in general:
    
    auto x = new Class();
    delete x;

* **Do use exceptions** for exceptional error conditions, not for normal program control flow.


