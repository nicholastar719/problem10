#include "Resistor.hpp"
#include <stdexcept>

void Resistor::set_value(double newval)
{
  if (newval < 0)
    throw std::invalid_argument("Resistance cannot be negative");
  Component::set_value(newval);
}

double Resistor::get_current() const
{
  double r = get_value();
  if (r == 0)
    throw std::logic_error("Resistance is 0");
  return get_voltage() / r;
}