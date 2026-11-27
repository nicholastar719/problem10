#ifndef VOLTAGESOURCE_HPP
#define VOLTAGESOURCE_HPP

#include "Component.hpp"

class VoltageSource : public Component
{
public:
  using Component::Component;
  double get_voltage() const override;
};

#endif