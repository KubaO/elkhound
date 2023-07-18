// restorer.h            see license.txt for copyright and terms of use
// A RAII value restorer utility.

#ifndef ELK_RESTORER_H
#define ELK_RESTORER_H

// ----------- automatic data value restorer -------------
// used when a value is to be set to one thing now, but restored
// to its original value on return (even when the return is by
// an exception being thrown)
template <class T>
class Restorer {
  T &variable;
  T prevValue;

  Restorer() = delete;
  Restorer(Restorer &) = delete;
  Restorer &operator=(Restorer &) = delete;

public:
  Restorer(T &var, T newValue)
    : variable(var),
    prevValue(var)
  {
    variable = std::move(newValue);
  }

  // this one does not set it to a new value, just remembers the current
  explicit Restorer(T &var)
    : variable(var),
    prevValue(var)
  {}

  ~Restorer()
  {
    variable = prevValue;
  }
};

#endif // ELK_RESTORER_H
