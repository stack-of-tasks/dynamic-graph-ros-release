// Copyright 2010 Thomas Moulard.
//

#include <string>

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include "signal-cast-register-test.h"
#include <dynamic-graph/debug.h>
#include <dynamic-graph/eigen-io.h>
#include <dynamic-graph/entity.h>
#include <dynamic-graph/factory.h>
#include <dynamic-graph/linear-algebra.h>
#include <dynamic-graph/pool.h>
#include <dynamic-graph/signal.h>

#include "signal-cast-registerer-libA.hh"
#include "signal-cast-registerer-libB.hh"

#define BOOST_TEST_MODULE signal_cast_registerer

#if BOOST_VERSION >= 105900
#include <boost/test/tools/output_test_stream.hpp>
#else
#include <boost/test/output_test_stream.hpp>
#endif
#include <boost/test/unit_test.hpp>
#include <iostream>

using boost::test_tools::output_test_stream;

typedef Eigen::VectorXd Vector;
typedef Eigen::MatrixXd Matrix;

// Check standard double cast registerer.
BOOST_AUTO_TEST_CASE(standard_double_registerer) {
  dynamicgraph::Signal<double, int> mySignal("out");

  typedef std::pair<std::string, std::string> test_t;
  std::vector<test_t> values;

  values.push_back(std::make_pair("42.0", "42"));
  values.push_back(std::make_pair("42.5", "42.5"));
  values.push_back(std::make_pair("-12.", "-12"));

  // Double special values.
  // FIXME: these tests are failing :(
  values.push_back(std::make_pair("inf", "inf"));
  values.push_back(std::make_pair("-inf", "-inf"));
  values.push_back(std::make_pair("nan", "nan"));

  BOOST_FOREACH (const test_t &test, values) {
    // Set
    std::istringstream value(test.first);
    mySignal.set(value);

    // Get
    {
      output_test_stream output;
      mySignal.get(output);
      BOOST_CHECK_EQUAL(output.str(), test.second);
    }

    // Trace
    {
      output_test_stream output;
      mySignal.trace(output);
      BOOST_CHECK_EQUAL(output.str(), test.second);
    }
  }

  // Check invalid values.
  // FIXME: this test is failing for now.
  std::istringstream value("This is not a valid double.");
  BOOST_CHECK_THROW(mySignal.set(value), std::exception);
}

// Check a custom cast registerer for Boost uBLAS vectors.
BOOST_AUTO_TEST_CASE(custom_vector_registerer) {
  dynamicgraph::Signal<dynamicgraph::Vector, int> myVectorSignal("vector");

  // Print the signal name.
  {
    output_test_stream output;
    output << myVectorSignal;
    BOOST_CHECK(output.is_equal("Sig:vector (Type Cst)"));
  }

  for (unsigned int i = 0; i < 5; ++i) {
    Vector v = Vector::Unit(5, i);
    std::ostringstream os;
    os << v;
    std::istringstream ss("[5](" + os.str() + ")");

    // Set signal value.
    myVectorSignal.set(ss);

    // Print out signal value.
    output_test_stream output;
    myVectorSignal.get(output);

    boost::format fmt("%d %d %d %d %d");
    fmt % (i == 0) % (i == 1) % (i == 2) % (i == 3) % (i == 4);

    BOOST_CHECK(output.is_equal(fmt.str()));
  }

  // Catch Exception of ss (not good input)

  // ss[0] != "["
  try {
    std::istringstream ss("test");
    myVectorSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[0] != \"[\"";
  }

  // ss[1] != %i
  try {
    std::istringstream ss("[test");
    myVectorSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[1] != %i";
  }

  // ss[2] != "]"
  try {
    std::istringstream ss("[5[");
    myVectorSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[2] != \"]\"";
  }

  // ss[3] != "("
  try {
    std::istringstream ss("[5]test");
    myVectorSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[3] != \"(\"";
  }

  // ss[4] != ' ' || ','
  try {
    std::istringstream ss("[5](1, ");
    myVectorSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[4] != \" \" || \",\"";
  }

  // ss[-1] != ")"
  try {
    std::istringstream ss("[5](1,2,3,4,5]");
    myVectorSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[-1] != \")\"";
  }

  try {
    output_test_stream output;
    myVectorSignal.trace(output);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[-1] != \")\"";
  }
}

BOOST_AUTO_TEST_CASE(custom_matrix_registerer) {
  dynamicgraph::Signal<dynamicgraph::Matrix, int> myMatrixSignal("matrix");

  // Print the signal name.
  {
    output_test_stream output;
    output << myMatrixSignal;
    BOOST_CHECK(output.is_equal("Sig:matrix (Type Cst)"));
  }

  // Catch Exception of ss (not good input)

  // ss[0] != "["
  try {
    std::istringstream ss("test");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[0] != \"[\"";
  }

  // ss[1] != %i
  try {
    std::istringstream ss("[test");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[1] != %i";
  }

  // ss[2] != ","
  try {
    std::istringstream ss("[5[");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[2] != \",\"";
  }

  // ss[3] != %i
  try {
    std::istringstream ss("[5,c");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[3] != %i";
  }

  // ss[4] != "]"
  try {
    std::istringstream ss("[5,3[");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[4] != \"]\"";
  }

  // ss[5] != "("
  try {
    std::istringstream ss("[5,3]test");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[5] != \"(\"";
  }

  // ss[6] != "("
  try {
    std::istringstream ss("[5,3](test");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[6] != \"(\"";
  }

  // ss[8] != " " || ","
  try {
    std::istringstream ss("[5,3]((1,");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[8] != \" \" || \",\"";
  }

  // ss[6+n] != ")"
  try {
    std::istringstream ss("[5,3]((1,2,3]");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << ("ss[6+n] != \")\"");
  }

  // ss[-3] != ")"
  try {
    std::istringstream ss("[5,1]((1)(2)(3[");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[5] != \")\"";
  }

  // ss[-3] != ")"
  try {
    std::istringstream ss("[5,1]((1)(2)(3)[");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[5] != \")\"";
  }

  // ss[-1]!= ")"
  try {
    std::istringstream ss("[3,1]((1)(2),(3)[");
    myMatrixSignal.set(ss);
  } catch (ExceptionSignal &e) {
    std::cout << "Test passed : ss[5] != \")\" and ignore \",\"";
  }

  //[...]((...))
}
