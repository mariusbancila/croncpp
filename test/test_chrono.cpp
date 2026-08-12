#include "catch.hpp"
#include "croncpp.h"

#include <chrono>
#include <string>
#include <type_traits>

using namespace cron;
using namespace std::chrono;

namespace
{
   system_clock::time_point point_at(std::string const & text)
   {
      auto date = utils::to_tm(text);

      return system_clock::from_time_t(utils::tm_to_time(date));
   }

   std::string text_of(system_clock::time_point const & tp)
   {
      auto const t = system_clock::to_time_t(tp);

      std::tm tm;
      if (utils::time_to_tm(&t, &tm) == nullptr) return "invalid";

      return utils::to_string(tm);
   }
}

TEST_CASE("chrono: the time_point overload agrees with the others", "[chrono]")
{
   auto cex = make_cron("0 15 10 * * ?");
   auto const from = point_at("2026-06-01 00:00:00");

   REQUIRE(text_of(cron_next(cex, from)) == "2026-06-01 10:15:00");

   // the three overloads name the same instant
   auto const from_time = system_clock::to_time_t(from);
   auto date = utils::to_tm("2026-06-01 00:00:00");

   auto from_date = cron_next(cex, date);

   REQUIRE(system_clock::to_time_t(cron_next(cex, from)) == cron_next(cex, from_time));
   REQUIRE(system_clock::to_time_t(cron_next(cex, from)) == utils::tm_to_time(from_date));
}

TEST_CASE("chrono: an expression with no next occurrence", "[chrono]")
{
   // the year has gone by, so there is nothing to return
   auto cex = make_cron("0 15 10 * * ? 2005");
   auto const from = point_at("2026-06-01 00:00:00");

   REQUIRE(cron_next(cex, from) == (system_clock::time_point::min)());
   REQUIRE(cron_next_ceil(cex, from) == (system_clock::time_point::min)());

   // and the time_t overload still reports it the way it always has
   REQUIRE(cron_next(cex, system_clock::to_time_t(from)) == INVALID_TIME);
}

TEST_CASE("chrono: the duration of the argument is kept", "[chrono]")
{
   auto cex = make_cron("0 15 10 * * ?");

   auto const in_ms = time_point_cast<milliseconds>(point_at("2026-06-01 00:00:00"));
   auto const out_ms = cron_next(cex, in_ms);

   static_assert(std::is_same<decltype(out_ms), time_point<system_clock, milliseconds> const>::value,
                 "milliseconds in, milliseconds out");
   REQUIRE(text_of(time_point_cast<system_clock::duration>(out_ms)) == "2026-06-01 10:15:00");

   auto const in_min = time_point_cast<minutes>(point_at("2026-06-01 00:00:00"));
   auto const out_min = cron_next(cex, in_min);

   static_assert(std::is_same<decltype(out_min), time_point<system_clock, minutes> const>::value,
                 "minutes in, minutes out");
   REQUIRE(text_of(time_point_cast<system_clock::duration>(out_min)) == "2026-06-01 10:15:00");
}

TEST_CASE("chrono: cron_next_ceil rounds up before searching", "[chrono]")
{
   auto cex = make_cron("0 0 12 * * *");
   auto const trigger = point_at("2026-06-01 12:00:00");

   // a shade below a trigger instant: truncation would put us back on it, so
   // cron_next answers with that trigger and cron_next_ceil with the next one
   auto const just_before = trigger - milliseconds(50);

   REQUIRE(text_of(cron_next(cex, just_before)) == "2026-06-01 12:00:00");
   REQUIRE(text_of(cron_next_ceil(cex, just_before)) == "2026-06-02 12:00:00");

   // exactly on the trigger, both move to the next one
   REQUIRE(text_of(cron_next(cex, trigger)) == "2026-06-02 12:00:00");
   REQUIRE(text_of(cron_next_ceil(cex, trigger)) == "2026-06-02 12:00:00");
}

TEST_CASE("chrono: the traits can still be given explicitly", "[chrono]")
{
   auto cex = make_cron<cron_quartz_traits>("0 15 10 ? * 6#2");
   auto const from = point_at("2011-04-30 23:30:00");

   REQUIRE(text_of(cron_next<cron_quartz_traits>(cex, from)) == "2011-05-13 10:15:00");
}
