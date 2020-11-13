/* Copyright 2019, LAAS-CNRS
 *
 * Olivier Stasse
 *
 * See LICENSE file
 *
 */
#include <iostream>
#include <sstream>

#define ENABLE_RT_LOG
#include "dynamic-graph/factory.h"
#include "dynamic-graph/pool.h"
#include <dynamic-graph/entity.h>
#include <dynamic-graph/exception-factory.h>

#include <dynamic-graph/logger.h>
#include <dynamic-graph/real-time-logger.h>

#define BOOST_TEST_MODULE debug - logger

#if BOOST_VERSION >= 105900
#include <boost/test/tools/output_test_stream.hpp>
#else
#include <boost/test/output_test_stream.hpp>
#endif
#include <boost/test/unit_test.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

using boost::test_tools::output_test_stream;

namespace dynamicgraph {
class CustomEntity : public Entity {
public:
  static const std::string CLASS_NAME;
  virtual const std::string &getClassName() const { return CLASS_NAME; }
  explicit CustomEntity(const std::string &n) : Entity(n) {
    logger_.setTimeSample(0.001);
    logger_.setStreamPrintPeriod(0.005);
    logger_.setVerbosity(VERBOSITY_ALL);
    LoggerVerbosity alv = logger_.getVerbosity();
    BOOST_CHECK(alv == VERBOSITY_ALL);
  }

  ~CustomEntity() {}
  void testDebugTrace() {
    sendMsg("This is a message of level MSG_TYPE_DEBUG", MSG_TYPE_DEBUG);
    sendMsg("This is a message of level MSG_TYPE_INFO", MSG_TYPE_INFO);
    sendMsg("This is a message of level MSG_TYPE_WARNING", MSG_TYPE_WARNING);
    sendMsg("This is a message of level MSG_TYPE_ERROR", MSG_TYPE_ERROR);
    sendMsg("This is a message of level MSG_TYPE_DEBUG_STREAM",
            MSG_TYPE_DEBUG_STREAM);
    sendMsg("This is a message of level MSG_TYPE_INFO_STREAM",
            MSG_TYPE_INFO_STREAM);
    sendMsg("This is a message of level MSG_TYPE_WARNING_STREAM",
            MSG_TYPE_WARNING_STREAM);
    sendMsg("This is a message of level MSG_TYPE_ERROR_STREAM",
            MSG_TYPE_ERROR_STREAM);

    logger_.countdown();
  }
};
DYNAMICGRAPH_FACTORY_ENTITY_PLUGIN(CustomEntity, "CustomEntity");
} // namespace dynamicgraph

BOOST_AUTO_TEST_CASE(debug_logger_wrong_initialization) {
  dynamicgraph::RealTimeLogger::instance();

  BOOST_CHECK_EQUAL(dynamicgraph::CustomEntity::CLASS_NAME, "CustomEntity");

  dynamicgraph::CustomEntity &entity =
      *(dynamic_cast<dynamicgraph::CustomEntity *>(
          dynamicgraph::FactoryStorage::getInstance()->newEntity(
              "CustomEntity", "my-entity-2")));

  for (unsigned int i = 0; i < 1000; i++) {
    entity.testDebugTrace();
  }

  dynamicgraph::RealTimeLogger::destroy();
}
