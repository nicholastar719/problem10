#include "PracticalVsCircuit.hpp"
#include <vector>
#include <cmath>
#include <stdexcept>

PracticalVsCircuit::PracticalVsCircuit(double vs, double rs, double rl)
    : VoltageDivider(vs, std::vector<double>{rs, rl}.data(), 2)
{
}

double PracticalVsCircuit::get_source_voltage() const
{
  return VoltageDivider::get_source_voltage() - get_resistor_voltage(0);
}

void PracticalVsCircuit::set_load(double rl)
{
  set_resistance(1, rl);
}

double PracticalVsCircuit::get_load_power() const
{
  double vl = get_resistor_voltage(1);
  return vl * vl / get_resistance(1);
}

bool PracticalVsCircuit::maximum_power_transfer(double rl)
{
  set_load(rl);
  double vs = VoltageDivider::get_source_voltage();
  double pmax = vs * vs / (4 * get_resistance(0));
  return std::fabs(get_load_power() - pmax) <= 1e-9;
}