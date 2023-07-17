// eval.h
// evaluation environment for gcom

#ifndef EVAL_H
#define EVAL_H

#include "str.h"          // string

#include <unordered_map>  // std::unordered_map

class Binding {
public:
  string name;
  int value;

public:
  Binding(char const *n, int v)
    : name(n),
      value(v)
  {}
  ~Binding();
};

class Env {
private:
  // map: name -> value
  std::unordered_map<string_view, Binding*> map;

public:
  Env();
  ~Env();

  int get(char const *x);
  void set(char const *x, int val);
};

#endif // ENV_H
