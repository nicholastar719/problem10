#ifndef CURRENTSOURCE_HPP
#define CURRENTSOURCE_HPP

#include "Component.hpp"

class CurrentSource : public Component
{
public:
  using Component::Component;
  double get_current() const override;
};

#endif