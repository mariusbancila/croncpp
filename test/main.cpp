#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "croncpp.h"

using namespace cron;

TEST_CASE("Test simple", "")
{
   auto c1 = make_cron("* * * * * *");
   REQUIRE(c1.seconds.to_string() == "111111111111111111111111111111111111111111111111111111111111");
   REQUIRE(c1.minutes.to_string() == "111111111111111111111111111111111111111111111111111111111111");
   REQUIRE(c1.hours.to_string() == "111111111111111111111111");
   REQUIRE(c1.days_of_week.to_string() == "1111111");
   REQUIRE(c1.days_of_month.to_string() == "1111111111111111111111111111111");
   REQUIRE(c1.months.to_string() == "111111111111");
}