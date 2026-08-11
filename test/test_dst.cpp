#include "catch.hpp"
#include "croncpp.h"

#include <cstdlib>
#include <ctime>
#include <string>

using namespace cron;

namespace
{
   // croncpp evaluates expressions in local time, so these tests pin the time
   // zone rather than depending on the one the machine happens to use.
   // CST6CDT is understood both by the Windows CRT, which applies the US rules
   // to it, and by the zoneinfo database used everywhere else. The POSIX rule
   // syntax, as in "CET-1CEST,M3.5.0,M10.5.0/3", is not portable to Windows.
   struct scoped_tz
   {
      explicit scoped_tz(char const * tz)
      {
#ifdef _WIN32
         _putenv_s("TZ", tz);
         _tzset();
#else
         setenv("TZ", tz, 1);
         tzset();
#endif
      }

      ~scoped_tz()
      {
#ifdef _WIN32
         _putenv_s("TZ", "");
         _tzset();
#else
         unsetenv("TZ");
         tzset();
#endif
      }
   };

   // in CST6CDT the clock jumps forward on 2025-03-09 at 02:00 and back on 2025-11-02 at 02:00
   char const * const TZ = "CST6CDT";

   std::time_t const NOV_02_0159_CDT = 1762066799; // first  01:59:59
   std::time_t const NOV_02_0159_CST = 1762070399; // second 01:59:59

   std::string next_from(char const * expr, char const * from)
   {
      auto cex = make_cron(expr);
      auto date = utils::to_tm(from);

      return utils::to_string(cron_next(cex, date));
   }
}

TEST_CASE("dst: the fixture selects a zone with the expected transitions", "[dst]")
{
   scoped_tz tz(TZ);

   std::tm first;
   std::tm second;
   REQUIRE(utils::time_to_tm(&NOV_02_0159_CDT, &first) != nullptr);
   REQUIRE(utils::time_to_tm(&NOV_02_0159_CST, &second) != nullptr);

   // the same local time, an hour apart, either side of the transition
   REQUIRE(utils::to_string(first) == "2025-11-02 01:59:59");
   REQUIRE(utils::to_string(second) == "2025-11-02 01:59:59");
   REQUIRE(first.tm_isdst > 0);
   REQUIRE(second.tm_isdst == 0);
}

TEST_CASE("dst: the clock going forward does not send the result backwards", "[dst]")
{
   scoped_tz tz(TZ);

   // asked on the day before the transition, for a time on the day of it
   REQUIRE(next_from("0 0 12 * * *", "2025-03-08 13:00:00") == "2025-03-09 12:00:00");
   REQUIRE(next_from("30 55 5,11,17 * * *", "2025-03-08 21:55:30") == "2025-03-09 05:55:30");

   // asked during the night of the transition, for a time just after it
   REQUIRE(next_from("30 55 3,11,17 * * *", "2025-03-09 00:55:30") == "2025-03-09 03:55:30");

   // the hour that does not exist: 02:30 is skipped, the next match is a day later
   REQUIRE(next_from("0 30 2 * * *", "2025-03-09 01:00:00") == "2025-03-10 02:30:00");
}

TEST_CASE("dst: the clock going back does not send the result backwards", "[dst]")
{
   scoped_tz tz(TZ);

   REQUIRE(next_from("0 30 1 * * *", "2025-11-01 12:00:00") == "2025-11-02 01:30:00");
   REQUIRE(next_from("0 0 12 * * *", "2025-11-01 13:00:00") == "2025-11-02 12:00:00");
}

TEST_CASE("dst: issue 24, a daily job asked for the day before the clock goes forward", "[dst]")
{
   scoped_tz tz(TZ);

   std::time_t const now = 1647084600; // 2022-03-12 05:30:00, transition on the 13th
   auto cex = make_cron("0 30 4 * * *");

   auto const next = cron_next(cex, now);

   REQUIRE(next > now);
   REQUIRE(next == 1647163800);        // 2022-03-13 04:30:00
}

TEST_CASE("dst: cron_next is always strictly in the future", "[dst]")
{
   scoped_tz tz(TZ);

   // Samples both transition nights against expressions that fire often
   // enough to land inside the transition. A result at or before the time
   // asked about would make a caller looping on cron_next spin, and that only
   // happens in a narrow window around a transition, so the window is swept
   // densely rather than spot checked.
   //
   // This is by far the largest test case in the suite: 5 expressions x 2
   // windows x 2058 samples (4 hours at 7 second steps) x 2 assertions =
   // 41160 assertions, of the roughly 42800 the whole suite reports. It still
   // runs in well under a second.
   char const * const exprs[] = {
      "0 * * * * *",
      "0 0/15 * * * *",
      "59 59 1 * * *",
      "0 30 2 * * *",
      "0 0 12 * * *",
   };

   std::time_t const spans[][2] = {
      { 1741500000, 1741500000 + 4 * 60 * 60 }, // around the spring transition
      { 1762063200, 1762063200 + 4 * 60 * 60 }, // around the autumn transition
   };

   for (auto const & expr : exprs)
   {
      auto cex = make_cron(expr);

      for (auto const & span : spans)
      {
         for (std::time_t t = span[0]; t < span[1]; t += 7)
         {
            auto const next = cron_next(cex, t);

            REQUIRE(next != INVALID_TIME);
            REQUIRE(next > t);
         }
      }
   }
}

TEST_CASE("dst: an ambiguous local time is resolved to a single instant", "[dst]")
{
   scoped_tz tz(TZ);

   // 01:59:59 happens twice on this date. Asked from the first one, croncpp
   // moves past the repeat rather than reporting the same local time again,
   // so a daily expression fires once on the day the clock goes back.
   auto cex = make_cron("59 59 1 * * *");
   auto const next = cron_next(cex, NOV_02_0159_CDT);

   REQUIRE(next > NOV_02_0159_CDT);

   std::tm next_tm;
   REQUIRE(utils::time_to_tm(&next, &next_tm) != nullptr);
   REQUIRE(next_tm.tm_hour == 1);
   REQUIRE(next_tm.tm_min == 59);
   REQUIRE(next_tm.tm_sec == 59);
}

TEST_CASE("dst: both overloads agree across a transition", "[dst]")
{
   scoped_tz tz(TZ);

   // Sweeps the hour before the transition, the repeated hour and the hour
   // after it: 3 expressions x 178 samples (3 hours at 61 second steps) x 2
   // assertions = 1068 assertions.
   char const * const exprs[] = { "0 * * * * *", "0 0 12 * * *", "59 59 1 * * *" };

   for (auto const & expr : exprs)
   {
      auto cex = make_cron(expr);

      for (std::time_t t = NOV_02_0159_CDT - 3600; t < NOV_02_0159_CST + 3600; t += 61)
      {
         std::tm date;
         REQUIRE(utils::time_to_tm(&t, &date) != nullptr);

         auto const from_time = cron_next(cex, t);
         auto from_tm = cron_next(cex, date);

         REQUIRE(from_time == utils::tm_to_time(from_tm));
      }
   }
}
