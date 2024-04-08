#include <doctest/doctest.h>
#include <zq/zq.hpp>
//#include <zq/typename.hpp>
#include "pingpong.pb.h"
#include <thread>
#include <chrono>
#include <tuple>

namespace {
  // startup times on Windows are a problem, they take too long,
  // this can cause test timeout
  using namespace std::chrono_literals;
  auto await_time = 1000ms;
  //auto await_time = std::chrono::milliseconds(1000);
}

// TUPLES NOT SUPORTED YET
//  because 'std::is_trivially_copyable_v<std::tuple<int, float, int> >' evaluated to false

SCENARIO("Send a struct") {

  auto context = zq::mk_context();

  GIVEN("a push and a pull socket") {
    // auto push = context->connect(zq::SocketType::PUSH, "tcp://localhost:5555");
    // auto pull = context->bind(zq::SocketType::PULL, "tcp://localhost:5555");

    auto push = context->connect(zq::SocketType::PUSH, "ipc://push_pull_test_tpl1");
    auto pull = context->bind(zq::SocketType::PULL, "ipc://push_pull_test_tpl1");

    REQUIRE(push);
    REQUIRE(pull);

    WHEN("pushing a tuple") {

      using mt = std::tuple<int, float, int>;
      auto t1 = std::make_tuple(1, 2.0f, 3);

      auto tm = zq::typed_message(t1);
      MESSAGE("sending: " << as_string(tm.type));
      auto res = push->send(tm);
      REQUIRE(res);

      THEN("it's possible to receive and restore the tuple") { // not yet
        auto reply = pull->await(await_time);
        REQUIRE(reply);
        auto restored = zq::restore_as<mt>(*reply.value());
        REQUIRE(restored);
        auto received = *restored;
        REQUIRE_EQ(received, t1);

      }
      AND_THEN("it's not possible to receive and restore a different tuple"){
        auto reply = pull->await(await_time);
        REQUIRE(reply);
        using other = std::tuple<int, float, float>;
        auto restored = zq::restore_as<other>(*reply.value());
        REQUIRE_FALSE(restored);
      }
    }
  }
}


