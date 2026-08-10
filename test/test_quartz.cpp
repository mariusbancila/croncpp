#include "catch.hpp"
#include "croncpp.h"

#include <string>

using namespace cron;

namespace
{
   template <typename Traits>
   std::tm next_of(std::string const & expr, std::string const & from)
   {
      auto cex = make_cron<Traits>(expr);
      return cron_next<Traits>(cex, utils::to_tm(from));
   }

   // 2023-09-03 is a Sunday, so the base date below covers the whole week
   // without crossing a month boundary.
   char const * const BASE = "2023-09-03 00:00:00";

   struct weekday_case
   {
      char const * name;
      char const * expected;
      int          wday;
   };

   weekday_case const WEEKDAYS[] = {
      { "SUN", "2023-09-03 12:00:00", 0 },
      { "MON", "2023-09-04 12:00:00", 1 },
      { "TUE", "2023-09-05 12:00:00", 2 },
      { "WED", "2023-09-06 12:00:00", 3 },
      { "THU", "2023-09-07 12:00:00", 4 },
      { "FRI", "2023-09-08 12:00:00", 5 },
      { "SAT", "2023-09-09 12:00:00", 6 },
   };
}

TEST_CASE("quartz: cron_next on a Sunday", "[quartz]")
{
   auto const res = next_of<cron_quartz_traits>("0 0 */12 * * 1-7", BASE);

   REQUIRE(utils::to_string(res) == "2023-09-03 12:00:00");
}

TEST_CASE("quartz: every day of the week is reachable", "[quartz]")
{
   for (auto const & c : WEEKDAYS)
   {
      auto const res = next_of<cron_quartz_traits>(
         std::string("0 0 12 ? * ") + c.name, BASE);

      REQUIRE(utils::to_string(res) == c.expected);
      REQUIRE(res.tm_wday == c.wday);
   }
}

TEST_CASE("quartz: named and numbered days of week agree", "[quartz]")
{
   // quartz numbers the days 1 (SUN) to 7 (SAT)
   for (int day = 0; day < 7; ++day)
   {
      auto const named = next_of<cron_quartz_traits>(
         std::string("0 0 12 ? * ") + WEEKDAYS[day].name, BASE);
      auto const numbered = next_of<cron_quartz_traits>(
         "0 0 12 ? * " + std::to_string(day + 1), BASE);

      REQUIRE(utils::to_string(numbered) == utils::to_string(named));
   }
}

TEST_CASE("quartz: days of week agree with the other traits", "[quartz]")
{
   // the same weekday, written in each numbering scheme, must resolve to the
   // same instant: standard is 0 (SUN) to 6 (SAT), quartz and oracle are
   // 1 (SUN) to 7 (SAT)
   for (int day = 0; day < 7; ++day)
   {
      auto const standard = next_of<cron_standard_traits>(
         "0 0 12 ? * " + std::to_string(day), BASE);
      auto const quartz = next_of<cron_quartz_traits>(
         "0 0 12 ? * " + std::to_string(day + 1), BASE);
      auto const oracle = next_of<cron_oracle_traits>(
         "0 0 12 ? * " + std::to_string(day + 1), BASE);

      REQUIRE(utils::to_string(standard) == WEEKDAYS[day].expected);
      REQUIRE(utils::to_string(quartz) == WEEKDAYS[day].expected);
      REQUIRE(utils::to_string(oracle) == WEEKDAYS[day].expected);
   }
}

TEST_CASE("quartz: day of week ranges", "[quartz]")
{
   // 2 (MON) to 6 (FRI) starting from a Sunday
   REQUIRE(utils::to_string(next_of<cron_quartz_traits>("0 0 12 ? * 2-6", BASE))
           == "2023-09-04 12:00:00");

   // starting from the Friday, the next weekday is the following Monday
   REQUIRE(utils::to_string(next_of<cron_quartz_traits>("0 0 12 ? * 2-6", "2023-09-08 12:00:01"))
           == "2023-09-11 12:00:00");

   // 7 (SAT) and 1 (SUN) only
   REQUIRE(utils::to_string(next_of<cron_quartz_traits>("0 0 12 ? * 1,7", "2023-09-04 00:00:00"))
           == "2023-09-09 12:00:00");
}
