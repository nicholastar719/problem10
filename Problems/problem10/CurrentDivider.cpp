#include "CurrentDivider.hpp"
#include <stdexcept>

// 2 nodes, components 0..n
// Component 0 = source, components 1..n = resistors
// Every component: A = node 0, B = node 1

CurrentDivider::CurrentDivider(double c, double res[], int num_res)
    : Network(2, num_res + 1)
{
  if (num_res < 1)
    throw std::invalid_argument("Need at least one resistor");

  CurrentSource *cs = new CurrentSource();
  set_component(0, cs);
  connect_component_terminal_a(0, 0);
  connect_component_terminal_b(0, 1);
  set_component_value(0, c);

  for (int k = 0; k < num_res; k++)
  {
    Resistor *r = new Resistor();
    set_component(k + 1, r);
    connect_component_terminal_a(k + 1, 0);
    connect_component_terminal_b(k + 1, 1);
    set_component_value(k + 1, res[k]);
  }
  update_node_voltages();
}

void CurrentDivider::update_node_voltages()
{
  int n = get_num_resistors();
  double conductance = 0;
  for (int k = 0; k < n; k++)
  {
    double r = get_component_value(k + 1);
    if (r == 0)
    {
      set_node_voltage(0, 0);
      set_node_voltage(1, 0);
      return;
    }
    conductance += 1.0 / r;
  }
  set_node_voltage(0, 0);
  set_node_voltage(1, get_component_value(0) / conductance);
}

int CurrentDivider::get_num_resistors() const
{
  return get_num_components() - 1;
}

double CurrentDivider::get_source_current() const
{
  return get_component_current(0);
}

double CurrentDivider::get_resistance(int idx) const
{
  if (idx < 0 || idx >= get_num_resistors())
    throw std::out_of_range("Invalid resistor index");
  return get_component_value(idx + 1);
}

double CurrentDivider::get_resistor_voltage() const
{
  return get_component_voltage(1);
}

double CurrentDivider::get_resistor_current(int idx) const
{
  if (idx < 0 || idx >= get_num_resistors())
    throw std::out_of_range("Invalid resistor index");
  return get_component_current(idx + 1);
}

void CurrentDivider::set_source_current(double c)
{
  set_component_value(0, c);
  update_node_voltages();
}

void CurrentDivider::set_resistance(int idx, double r)
{
  if (idx < 0 || idx >= get_num_resistors())
    throw std::out_of_range("Invalid resistor index");
  set_component_value(idx + 1, r);
  update_node_voltages();
}