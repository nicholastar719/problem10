#include "VoltageDivider.hpp"
#include <stdexcept>

// n resistors -> nodes 0..n, components 0..n
// Component 0 = source (A = node 0, B = node 1)
// Component i+1 = resistor i (B = node i+1, A = node i+2; last resistor's A = node 0)

VoltageDivider::VoltageDivider(double v, double res[], int num_res)
    : Network(num_res + 1, num_res + 1)
{
  if (num_res < 1)
    throw std::invalid_argument("Need at least one resistor");

  VoltageSource *vs = new VoltageSource();
  set_component(0, vs);
  connect_component_terminal_a(0, 0);
  connect_component_terminal_b(0, 1);
  set_component_value(0, v);

  for (int i = 0; i < num_res; i++)
  {
    Resistor *r = new Resistor();
    set_component(i + 1, r);
    connect_component_terminal_b(i + 1, i + 1);
    connect_component_terminal_a(i + 1, (i + 1 == num_res) ? 0 : i + 2);
    set_component_value(i + 1, res[i]);
  }
  update_node_voltages();
}

void VoltageDivider::update_node_voltages()
{
  int n = get_num_resistors();
  double vs = get_component_value(0);
  double total = 0;
  for (int i = 0; i < n; i++)
    total += get_component_value(i + 1);

  double current = (total == 0) ? 0 : vs / total;

  set_node_voltage(0, 0);
  double v = vs;
  set_node_voltage(1, v);
  for (int k = 1; k < n; k++)
  {
    v -= current * get_component_value(k); // drop across resistor k-1
    set_node_voltage(k + 1, v);
  }
}

int VoltageDivider::get_num_resistors() const
{
  return get_num_components() - 1;
}

double VoltageDivider::get_source_voltage() const
{
  return get_component_voltage(0);
}

double VoltageDivider::get_resistance(int idx) const
{
  if (idx < 0 || idx >= get_num_resistors())
    throw std::out_of_range("Invalid resistor index");
  return get_component_value(idx + 1);
}

double VoltageDivider::get_resistor_voltage(int idx) const
{
  if (idx < 0 || idx >= get_num_resistors())
    throw std::out_of_range("Invalid resistor index");
  return get_component_voltage(idx + 1);
}

double VoltageDivider::get_loop_current() const
{
  return get_component_current(1);
}

void VoltageDivider::set_source_voltage(double v)
{
  set_component_value(0, v);
  update_node_voltages();
}

void VoltageDivider::set_resistance(int idx, double r)
{
  if (idx < 0 || idx >= get_num_resistors())
    throw std::out_of_range("Invalid resistor index");
  set_component_value(idx + 1, r);
  update_node_voltages();
}