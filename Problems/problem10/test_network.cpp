#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <stdexcept>
#include <cmath>

#include "Node.hpp"
#include "Component.hpp"
#include "VoltageSource.hpp"
#include "CurrentSource.hpp"
#include "Resistor.hpp"

#include "Network.hpp"
#include "VoltageDivider.hpp"
#include "CurrentDivider.hpp"
#include "PracticalVsCircuit.hpp"

const double THRESHOLD = 1e-9;

bool compare_double(double val1, double val2)
{
  double diff = std::abs(val1 - val2);
  return (diff <= THRESHOLD);
}

// VoltageSource class unit tests
TEST_CASE("Network: Test VoltageSource", "[network]")
{
  VoltageSource vs;

  // test inherited behavior
  REQUIRE(vs.get_node_a() == nullptr);
  REQUIRE(vs.get_node_b() == nullptr);

  Node *node1ptr = new Node();
  Node *node2ptr = new Node();
  REQUIRE_NOTHROW(vs.set_node_a(node1ptr));
  REQUIRE_NOTHROW(vs.set_node_b(node2ptr));
  REQUIRE(vs.get_node_a() == node1ptr);
  REQUIRE(vs.get_node_b() == node2ptr);
  REQUIRE(compare_double(vs.get_voltage(), 0));

  // test source voltage, should start as 0 from Component default
  double voltage = 3.3;
  REQUIRE(compare_double(vs.get_voltage(), 0));
  vs.set_value(voltage);
  // source voltage should be independent of node voltages
  REQUIRE(compare_double(vs.get_voltage(), voltage));

  // construct with pointers, test that source voltage is separate from terminal voltages
  double v1 = 5.2, v2 = 1.3;
  node1ptr->set_voltage(v1);
  node2ptr->set_voltage(v2);
  VoltageSource vs2(node1ptr, node2ptr);
  REQUIRE(compare_double(vs2.get_voltage(), 0));
  vs2.set_value(-voltage);
  REQUIRE(compare_double(vs2.get_voltage(), -voltage));

  delete node1ptr;
  delete node2ptr;
}

// CurrentSource class unit tests
TEST_CASE("Network: Test CurrentSource", "[network]")
{
  // test inherited behavior
  CurrentSource cs;

  REQUIRE(cs.get_node_a() == nullptr);
  REQUIRE(cs.get_node_b() == nullptr);
  REQUIRE_THROWS_AS(cs.get_voltage(), std::logic_error);

  Node *node1ptr = new Node();
  Node *node2ptr = new Node();
  REQUIRE_NOTHROW(cs.set_node_a(node1ptr));
  REQUIRE_NOTHROW(cs.set_node_b(node2ptr));
  REQUIRE(cs.get_node_a() == node1ptr);
  REQUIRE(cs.get_node_b() == node2ptr);
  REQUIRE(compare_double(cs.get_voltage(), 0));

  // test source current, should start as 0 from Component default
  double current = 3.3;
  REQUIRE(compare_double(cs.get_current(), 0));
  cs.set_value(current);
  REQUIRE(compare_double(cs.get_current(), current));

  // construct with pointers, test current and voltage
  double v1 = 5.2, v2 = 1.3;
  node1ptr->set_voltage(v1);
  node2ptr->set_voltage(v2);
  CurrentSource cs2(node1ptr, node2ptr);
  REQUIRE(compare_double(cs2.get_current(), 0));
  cs2.set_value(-current);
  REQUIRE(compare_double(cs2.get_voltage(), v2 - v1));
  REQUIRE(compare_double(cs2.get_current(), -current));

  delete node1ptr;
  delete node2ptr;
}

// Resistor class unit tests
TEST_CASE("Network: Test Resistor", "[network]")
{
  // test inheritance
  Resistor r;

  REQUIRE(r.get_node_a() == nullptr);
  REQUIRE(r.get_node_b() == nullptr);
  REQUIRE_THROWS_AS(r.get_voltage(), std::logic_error);

  Node *node1ptr = new Node();
  Node *node2ptr = new Node();
  REQUIRE_NOTHROW(r.set_node_a(node1ptr));
  REQUIRE_NOTHROW(r.set_node_b(node2ptr));
  REQUIRE(r.get_node_a() == node1ptr);
  REQUIRE(r.get_node_b() == node2ptr);
  REQUIRE(compare_double(r.get_voltage(), 0));

  // test resistance, should start as 0 from Component default
  double resistance = 590;
  REQUIRE(compare_double(r.get_value(), 0));
  r.set_value(0); // 0 resistance is allowed
  r.set_value(resistance);
  REQUIRE(compare_double(r.get_value(), resistance));

  // test negative resistance
  REQUIRE_THROWS_AS(r.set_value(-resistance), std::invalid_argument);

  // test current
  double v1 = 5.2, v2 = 1.3;
  node1ptr->set_voltage(v1);
  node2ptr->set_voltage(v2);
  REQUIRE(compare_double(r.get_current(), (v2 - v1) / resistance));

  // test get_current throws
  r.set_node_a(nullptr);
  // node a nullptr
  REQUIRE_THROWS_AS(r.get_current(), std::logic_error);
  r.set_node_b(nullptr);
  // both nodes nullptr
  REQUIRE_THROWS_AS(r.get_current(), std::logic_error);
  r.set_node_a(node1ptr);
  // node b still nullptr
  REQUIRE_THROWS_AS(r.get_current(), std::logic_error);
  r.set_node_b(node2ptr);
  // valid
  REQUIRE(compare_double(r.get_current(), (v2 - v1) / resistance));
  // div by 0
  r.set_value(0);
  REQUIRE_THROWS_AS(r.get_current(), std::logic_error);

  delete node1ptr;
  delete node2ptr;
}

// General unit tests of overriding and dynamic binding
TEST_CASE("Network: Test overriding and dynamic binding", "[network]")
{
  Component c;
  Resistor r;
  Component *c_ptr = new Resistor();

  REQUIRE_NOTHROW(c.set_value(-1));
  REQUIRE_THROWS_AS(r.set_value(-1), std::invalid_argument);
  REQUIRE_THROWS_AS(c_ptr->set_value(-1), std::invalid_argument);

  c.set_value(0);
  r.set_value(0);
  c_ptr->set_value(0);
  REQUIRE_NOTHROW(c.get_current());
  REQUIRE_THROWS_AS(r.get_current(), std::logic_error);
  REQUIRE_THROWS_AS(c_ptr->get_current(), std::logic_error);

  double val = 0.1;
  Component c_base(val);
  VoltageSource vs(val);
  CurrentSource cs(val);
  Component *c_ptr1 = new VoltageSource(val);
  Component *c_ptr2 = new CurrentSource(val);

  REQUIRE(compare_double(c_base.get_current(), 0.0));
  REQUIRE(compare_double(vs.get_current(), 0.0));
  REQUIRE(compare_double(cs.get_current(), val));
  REQUIRE(compare_double(c_ptr1->get_current(), 0.0));
  REQUIRE(compare_double(c_ptr2->get_current(), val));

  REQUIRE_THROWS_AS(c_base.get_voltage(), std::logic_error);
  REQUIRE(compare_double(vs.get_voltage(), val));
  REQUIRE_THROWS_AS(cs.get_voltage(), std::logic_error);
  REQUIRE(compare_double(c_ptr1->get_voltage(), val));
  REQUIRE_THROWS_AS(c_ptr2->get_voltage(), std::logic_error);

  delete c_ptr;
  delete c_ptr1;
  delete c_ptr2;
}

// Write your own test cases here.

TEST_CASE("Network: Test VoltageDivider", "[network]")
{
  double voltage = 5.0;
  const int num = 2;
  double res[num] = {200, 400};
  VoltageDivider div(voltage, res, num);

  // structure: ground + source node + 1 node between resistors
  REQUIRE(div.get_num_resistors() == 2);
  REQUIRE(div.get_num_nodes() == 3);
  REQUIRE(div.get_num_components() == 3);

  // values
  REQUIRE(compare_double(div.get_source_voltage(), 5.0));
  REQUIRE(compare_double(div.get_resistance(0), 200));
  REQUIRE(compare_double(div.get_resistance(1), 400));

  // node voltages
  REQUIRE(compare_double(div.get_node_voltage(0), 0));
  REQUIRE(compare_double(div.get_node_voltage(1), 5.0));
  REQUIRE(compare_double(div.get_node_voltage(2), 5.0 * 400 / 600));

  // resistor voltages and loop current
  REQUIRE(compare_double(div.get_resistor_voltage(0), 5.0 * 200 / 600));
  REQUIRE(compare_double(div.get_resistor_voltage(1), 5.0 * 400 / 600));
  REQUIRE(compare_double(div.get_loop_current(), 5.0 / 600));

  // change source voltage
  div.set_source_voltage(10.0);
  REQUIRE(compare_double(div.get_source_voltage(), 10.0));
  REQUIRE(compare_double(div.get_resistor_voltage(1), 10.0 * 400 / 600));
  REQUIRE(compare_double(div.get_loop_current(), 10.0 / 600));

  // change a resistance
  div.set_resistance(0, 600);
  REQUIRE(compare_double(div.get_resistance(0), 600));
  REQUIRE(compare_double(div.get_resistor_voltage(0), 10.0 * 600 / 1000));
  REQUIRE(compare_double(div.get_resistor_voltage(1), 10.0 * 400 / 1000));
  REQUIRE(compare_double(div.get_loop_current(), 10.0 / 1000));

  // exceptions
  REQUIRE_THROWS_AS(div.get_resistance(-1), std::out_of_range);
  REQUIRE_THROWS_AS(div.get_resistance(2), std::out_of_range);
  REQUIRE_THROWS_AS(div.get_resistor_voltage(2), std::out_of_range);
  REQUIRE_THROWS_AS(div.set_resistance(2, 100), std::out_of_range);
  REQUIRE_THROWS_AS(div.set_resistance(0, -100), std::invalid_argument);
  REQUIRE_THROWS_AS(VoltageDivider(5.0, res, 0), std::invalid_argument);
  double bad[1] = {-10};
  REQUIRE_THROWS_AS(VoltageDivider(5.0, bad, 1), std::invalid_argument);
}

TEST_CASE("Network: Test CurrentDivider", "[network]")
{
  double current = 0.006;
  const int num = 2;
  double res[num] = {15000, 75000};
  CurrentDivider div(current, res, num);

  REQUIRE(div.get_num_resistors() == 2);
  REQUIRE(div.get_num_nodes() == 2);
  REQUIRE(div.get_num_components() == 3);

  REQUIRE(compare_double(div.get_source_current(), 0.006));
  REQUIRE(compare_double(div.get_resistance(0), 15000));
  REQUIRE(compare_double(div.get_resistance(1), 75000));

  // parallel resistance = 12500, so V = 0.006 * 12500 = 75
  REQUIRE(compare_double(div.get_node_voltage(0), 0));
  REQUIRE(compare_double(div.get_node_voltage(1), 75.0));
  REQUIRE(compare_double(div.get_resistor_voltage(), 75.0));
  REQUIRE(compare_double(div.get_resistor_current(0), 0.005));
  REQUIRE(compare_double(div.get_resistor_current(1), 0.001));

  // change source current
  div.set_source_current(0.012);
  REQUIRE(compare_double(div.get_source_current(), 0.012));
  REQUIRE(compare_double(div.get_resistor_voltage(), 150.0));
  REQUIRE(compare_double(div.get_resistor_current(0), 0.010));

  // change a resistance (both 15000: parallel = 7500, V = 0.012 * 7500 = 90)
  div.set_resistance(1, 15000);
  REQUIRE(compare_double(div.get_resistance(1), 15000));
  REQUIRE(compare_double(div.get_resistor_voltage(), 90.0));
  REQUIRE(compare_double(div.get_resistor_current(1), 0.006));

  // exceptions
  REQUIRE_THROWS_AS(div.get_resistance(2), std::out_of_range);
  REQUIRE_THROWS_AS(div.get_resistor_current(-1), std::out_of_range);
  REQUIRE_THROWS_AS(div.get_resistor_current(2), std::out_of_range);
  REQUIRE_THROWS_AS(div.set_resistance(2, 100), std::out_of_range);
  REQUIRE_THROWS_AS(div.set_resistance(0, -100), std::invalid_argument);
  REQUIRE_THROWS_AS(CurrentDivider(0.006, res, 0), std::invalid_argument);
}

TEST_CASE("Network: Test PracticalVsCircuit", "[network]")
{
  // VS = 10, RS = 5, RL = 5
  PracticalVsCircuit p(10.0, 5.0, 5.0);
  REQUIRE(compare_double(p.get_source_voltage(), 5.0));  // terminal voltage
  REQUIRE(compare_double(p.get_load_power(), 5.0));      // 5^2 / 5

  // change load: RL = 20, VL = 10 * 20 / 25 = 8, PL = 64 / 20 = 3.2
  p.set_load(20.0);
  REQUIRE(compare_double(p.get_source_voltage(), 8.0));
  REQUIRE(compare_double(p.get_load_power(), 3.2));

  // maximum power transfer
  REQUIRE(p.maximum_power_transfer(5.0));
  REQUIRE_FALSE(p.maximum_power_transfer(20.0));
  REQUIRE(compare_double(p.get_resistance(1), 20.0));  // the call changed the load

  // exceptions
  REQUIRE_THROWS_AS(p.set_load(-1.0), std::invalid_argument);
  REQUIRE_THROWS_AS(p.maximum_power_transfer(-1.0), std::invalid_argument);
  REQUIRE_THROWS_AS(PracticalVsCircuit(10.0, -5.0, 5.0), std::invalid_argument);
  REQUIRE_THROWS_AS(PracticalVsCircuit(10.0, 5.0, -5.0), std::invalid_argument);
}

TEST_CASE("Network: Test PracticalVsCircuit binding", "[network]")
{
  // dynamic vs. static binding through a VoltageDivider pointer
  VoltageDivider *vd_ptr = new PracticalVsCircuit(10.0, 5.0, 5.0);
  REQUIRE(compare_double(vd_ptr->get_source_voltage(), 5.0));                  // dynamic: terminal voltage
  REQUIRE(compare_double(vd_ptr->VoltageDivider::get_source_voltage(), 10.0)); // static: ideal voltage
  delete vd_ptr;

  // through a Network pointer (safe delete because ~Network is virtual)
  Network *net_ptr = new PracticalVsCircuit(10.0, 5.0, 5.0);
  REQUIRE(compare_double(net_ptr->get_node_voltage(1), 10.0));
  REQUIRE(compare_double(net_ptr->get_node_voltage(2), 5.0));
  delete net_ptr;

  // reference and pointer to the same Resistor
  Resistor r;
  Component &ref = r;
  Component *ptr = &r;
  REQUIRE_THROWS_AS(ref.set_value(-1), std::invalid_argument);
  REQUIRE_THROWS_AS(ptr->set_value(-1), std::invalid_argument);
}