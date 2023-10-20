# Vandevoorde, Josuttis, Gregor - C++ Templates - The Complete Guide

* To accesss members of base classes that depend on template parameters, you have to qualify the access by `this->` or their class name
```
template<typename T>
class Base {
public:
  void bar();
};

template<typename T>
class Derived : Base<T> {
public:
  void foo() {
    bar(); //this->bar(); or Base<T>::bar(); otherwise a global bar() function will be called
  }
};
```
