#include "catch.hpp"
#include "croncpp.h"

#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

using namespace cron;

namespace
{
   // croncpp evaluates expressions in local time, so these tests pin the time
   // zone rather than depending on the one the machine happens to use.
   // CST6CDT is read as a zone name where the zoneinfo database has the
   // compatibility names installed, and as a POSIX specification otherwise.
   //
   // The dates it switches on are NOT the same everywhere: the Windows CRT
   // applies its own idea of the US rules and does not agree with zoneinfo.
   // Nothing below assumes a date. The transitions are discovered from the C
   // runtime, and every expectation is expressed relative to them.
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

   char const * const TZ = "CST6CDT";

   int const YEAR = 2025;

   std::tm local_at(std::time_t const t)
   {
      std::tm tm;
      REQUIRE(utils::time_to_tm(&t, &tm) != nullptr);

      return tm;
   }

   int isdst_at(std::time_t const t)
   {
      std::tm tm;
      if (utils::time_to_tm(&t, &tm) == nullptr) return -1;

      return tm.tm_isdst > 0 ? 1 : 0;
   }

   struct transition
   {
      std::time_t at;      // the first instant on the new side of the change
      bool        forward; // the clock jumped forward, skipping local times
   };

   // Walks a year an hour at a time looking for the DST flag changing, then
   // narrows each change down to the second.
   std::vector<transition> transitions_in(int const year)
   {
      std::vector<transition> found;

      std::tm start = std::tm();
      start.tm_year = year - 1900;
      start.tm_mon = 0;
      start.tm_mday = 1;
      start.tm_hour = 12;
      start.tm_isdst = -1;

      std::time_t const first = utils::tm_to_time(start);
      if (INVALID_TIME == first) return found;

      std::time_t const last = first + 364 * 24 * 60 * 60;

      int previous = isdst_at(first);
      for (std::time_t t = first + 3600; t <= last; t += 3600)
      {
         int const current = isdst_at(t);
         if (current == previous) continue;

         for (std::time_t s = t - 3599; s <= t; ++s)
         {
            if (isdst_at(s) != previous)
            {
               transition const change = { s, current == 1 };
               found.push_back(change);
               break;
            }
         }

         previous = current;
      }

      return found;
   }

   std::string daily_at(int const hour, int const minute)
   {
      return "0 " + std::to_string(minute) + " " + std::to_string(hour) + " * * *";
   }
}

TEST_CASE("dst: the zone under test observes daylight saving time", "[dst]")
{
   scoped_tz tz(TZ);

   auto const changes = transitions_in(YEAR);

   INFO("the zone " << TZ << " reported " << changes.size() << " transitions in " << YEAR);
   REQUIRE(changes.size() >= 2);

   bool forward = false;
   bool back = false;
   for (size_t i = 0; i < changes.size(); ++i)
   {
      if (changes[i].forward) forward = true;
      else                    back = true;
   }

   REQUIRE(forward);
   REQUIRE(back);
}

TEST_CASE("dst: the hour the clock skips does not fire that day", "[dst]")
{
   scoped_tz tz(TZ);

   auto const changes = transitions_in(YEAR);
   REQUIRE(!changes.empty());

   for (size_t i = 0; i < changes.size(); ++i)
   {
      if (!changes[i].forward) continue;

      std::tm const before = local_at(changes[i].at - 1);
      std::tm const after = local_at(changes[i].at);

      // only a whole hour skipped on the hour is covered here
      if (after.tm_hour != before.tm_hour + 2) continue;

      int const skipped = before.tm_hour + 1;

      auto cex = make_cron(daily_at(skipped, 30));
      std::time_t const from = changes[i].at - 3600;

      auto const next = cron_next(cex, from);
      REQUIRE(next > from);

      std::tm const tm = local_at(next);
      REQUIRE(tm.tm_hour == skipped);
      REQUIRE(tm.tm_min == 30);
      REQUIRE(tm.tm_mday != before.tm_mday); // not on the day the hour is missing
   }
}

TEST_CASE("dst: issue 24, a daily job keeps advancing a day at a time", "[dst]")
{
   scoped_tz tz(TZ);

   auto const changes = transitions_in(YEAR);
   REQUIRE(!changes.empty());

   for (size_t i = 0; i < changes.size(); ++i)
   {
      // a time of day that exists on both sides of the change
      int const hour = (local_at(changes[i].at).tm_hour + 1) % 24;

      auto cex = make_cron(daily_at(hour, 30));

      std::time_t t = changes[i].at - 3 * 24 * 60 * 60;
      for (int day = 0; day < 6; ++day)
      {
         auto const next = cron_next(cex, t);

         // the reported symptom was a result at or before the time asked about
         REQUIRE(next > t);
         REQUIRE(next - t <= 25 * 60 * 60); // and never a day skipped

         std::tm const tm = local_at(next);
         REQUIRE(tm.tm_hour == hour);
         REQUIRE(tm.tm_min == 30);

         t = next;
      }
   }
}

TEST_CASE("dst: an ambiguous local time resolves to a single instant", "[dst]")
{
   scoped_tz tz(TZ);

   auto const changes = transitions_in(YEAR);
   REQUIRE(!changes.empty());

   for (size_t i = 0; i < changes.size(); ++i)
   {
      if (changes[i].forward) continue;

      // the local times in the hour before the change happen a second time in
      // the hour after it
      std::tm const repeated = local_at(changes[i].at - 1);

      auto cex = make_cron(daily_at(repeated.tm_hour, repeated.tm_min));
      std::time_t const from = changes[i].at - 3600;

      auto const next = cron_next(cex, from);
      REQUIRE(next > from);

      std::tm const tm = local_at(next);
      REQUIRE(tm.tm_hour == repeated.tm_hour);
      REQUIRE(tm.tm_min == repeated.tm_min);
   }
}

TEST_CASE("dst: cron_next is always strictly in the future", "[dst]")
{
   scoped_tz tz(TZ);

   auto const changes = transitions_in(YEAR);
   REQUIRE(!changes.empty());

   // Sweeps two hours either side of every transition, against expressions
   // that fire often enough to land inside one. A result at or before the time
   // asked about would make a caller looping on cron_next spin, and that only
   // happens in a narrow window around a transition, so the window is swept
   // densely rather than spot checked.
   //
   // This is by far the largest test case in the suite: 5 expressions x 2
   // transitions x 2058 samples (4 hours at 7 second steps) x 2 assertions =
   // 41160 assertions, of the roughly 43500 the whole suite reports. It still
   // runs in a fraction of a second.
   char const * const exprs[] = {
      "0 * * * * *",
      "0 0/15 * * * *",
      "59 59 1 * * *",
      "0 30 2 * * *",
      "0 0 12 * * *",
   };

   for (auto const & expr : exprs)
   {
      auto cex = make_cron(expr);

      for (size_t i = 0; i < changes.size(); ++i)
      {
         std::time_t const stop = changes[i].at + 2 * 60 * 60;
         for (std::time_t t = changes[i].at - 2 * 60 * 60; t < stop; t += 7)
         {
            auto const next = cron_next(cex, t);

            REQUIRE(next != INVALID_TIME);
            REQUIRE(next > t);
         }
      }
   }
}

TEST_CASE("dst: both overloads agree across a transition", "[dst]")
{
   scoped_tz tz(TZ);

   auto const changes = transitions_in(YEAR);
   REQUIRE(!changes.empty());

   // Asking with a std::time_t and with the equivalent std::tm has to name the
   // same instant, including where the local time is ambiguous and mktime has
   // to choose between two readings of it.
   //
   // 3 expressions x 2 transitions x 178 samples (3 hours at 61 second steps)
   // x 2 assertions = 2136 assertions.
   char const * const exprs[] = { "0 * * * * *", "0 0 12 * * *", "59 59 1 * * *" };

   for (auto const & expr : exprs)
   {
      auto cex = make_cron(expr);

      for (size_t i = 0; i < changes.size(); ++i)
      {
         std::time_t const stop = changes[i].at + 2 * 60 * 60;
         for (std::time_t t = changes[i].at - 60 * 60; t < stop; t += 61)
         {
            std::tm date;
            REQUIRE(utils::time_to_tm(&t, &date) != nullptr);

            auto const from_time = cron_next(cex, t);
            auto from_tm = cron_next(cex, date);

            REQUIRE(from_time == utils::tm_to_time(from_tm));
         }
      }
   }
}
