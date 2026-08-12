#include "catch.hpp"
#include "croncpp.h"

#include <chrono>
#include <string>

using namespace cron;

namespace
{
   std::time_t time_at(std::string const & text)
   {
      auto date = utils::to_tm(text);

      return utils::tm_to_time(date);
   }

   std::string text_at(std::time_t const t)
   {
      std::tm tm;
      if (utils::time_to_tm(&t, &tm) == nullptr) return "invalid";

      return utils::to_string(tm);
   }

   template <typename Traits>
   std::string prev_str(std::string const & expr, std::string const & from)
   {
      auto cex = make_cron<Traits>(expr);
      auto const result = cron_prev<Traits>(cex, time_at(from));

      return INVALID_TIME == result ? "none" : text_at(result);
   }

   std::string prev(std::string const & expr, std::string const & from)
   {
      return prev_str<cron_standard_traits>(expr, from);
   }
}

TEST_CASE("prev: the occurrence before a given time", "[prev]")
{
   REQUIRE(prev("0 15 10 * * ?", "2026-06-01 12:00:00") == "2026-06-01 10:15:00");
   REQUIRE(prev("0 15 10 * * ?", "2026-06-01 09:00:00") == "2026-05-31 10:15:00");
   REQUIRE(prev("0 0 12 * * *", "2026-06-01 12:00:01") == "2026-06-01 12:00:00");
}

TEST_CASE("prev: strictly earlier, as cron_next is strictly later", "[prev]")
{
   // asked at an occurrence, the answer is the one before it rather than itself
   REQUIRE(prev("0 15 10 * * ?", "2026-06-01 10:15:00") == "2026-05-31 10:15:00");
   REQUIRE(prev("* * * * * *", "2026-06-01 10:15:00") == "2026-06-01 10:14:59");
}

TEST_CASE("prev: sparse expressions", "[prev]")
{
   REQUIRE(prev("0 0 0 29 2 *", "2026-06-01 00:00:00") == "2024-02-29 00:00:00");
   REQUIRE(prev("0 0 0 1 1 ?", "2026-06-01 00:00:00") == "2026-01-01 00:00:00");
   REQUIRE(prev("0 15 10 ? * 5L", "2026-06-01 00:00:00") == "2026-05-29 10:15:00");
   REQUIRE(prev("0 15 10 L * ?", "2026-06-01 00:00:00") == "2026-05-31 10:15:00");
}

TEST_CASE("prev: an expression with a great many occurrences in the interval", "[prev]")
{
   // every second of the 1st of January. Walking the year's occurrences one at
   // a time would be 86400 steps; halving the interval is a few dozen.
   REQUIRE(prev("* * * 1 1 ?", "2026-06-01 00:00:00") == "2026-01-01 23:59:59");
   REQUIRE(prev("* * * 1 1 ?", "2026-01-01 12:00:00") == "2026-01-01 11:59:59");
}

TEST_CASE("prev: expressions with a year", "[prev]")
{
   REQUIRE(prev("0 15 10 * * ? 2005", "2006-01-01 00:00:00") == "2005-12-31 10:15:00");
   REQUIRE(prev("0 15 10 * * ? 2005", "2005-06-01 00:00:00") == "2005-05-31 10:15:00");

   // nothing has happened yet, so there is nothing before it
   REQUIRE(prev("0 15 10 * * ? 2050", "2026-06-01 00:00:00") == "none");

   // beyond the four year horizon the search reaches back to
   REQUIRE(prev("0 15 10 * * ? 2005", "2026-06-01 00:00:00") == "none");
}

TEST_CASE("prev: it agrees with cron_next", "[prev]")
{
   // whatever the expression, the occurrence before a time is earlier than it,
   // and the occurrence after that one is not
   char const * const exprs[] = {
      "* * * * * *",
      "0 * * * * *",
      "0 0/15 * * * *",
      "0 15 10 * * ?",
      "0 0 0 1 * ?",
      "0 15 10 ? * 5#2",
      "0 0 0 29 2 *",
   };

   std::time_t const from = time_at("2026-06-15 13:47:11");

   for (auto const & expr : exprs)
   {
      auto cex = make_cron(expr);

      std::time_t at = from;
      for (int step = 0; step < 5; ++step)
      {
         auto const previous = cron_prev(cex, at);
         REQUIRE(previous != INVALID_TIME);
         REQUIRE(previous < at);

         // nothing matches between the two
         REQUIRE(cron_next(cex, previous) >= at);

         at = previous;
      }
   }
}

TEST_CASE("prev: the std::tm overload", "[prev]")
{
   auto cex = make_cron("0 15 10 * * ?");
   auto date = utils::to_tm("2026-06-01 12:00:00");

   auto result = cron_prev(cex, date);
   REQUIRE(utils::to_string(result) == "2026-06-01 10:15:00");

   // a zeroed tm when there is nothing before the time given
   auto future = make_cron("0 15 10 * * ? 2050");
   auto none = cron_prev(future, date);
   REQUIRE(none.tm_year == 0);
   REQUIRE(none.tm_mday == 0);
}

TEST_CASE("prev: the time_point overload", "[prev]")
{
   using namespace std::chrono;

   auto cex = make_cron("0 0 12 * * *");
   auto const noon = system_clock::from_time_t(time_at("2026-06-01 12:00:00"));

   REQUIRE(text_at(system_clock::to_time_t(cron_prev(cex, noon))) == "2026-05-31 12:00:00");

   // a time point just past an occurrence still has that occurrence behind it,
   // which truncation to whole seconds would otherwise hide
   auto const just_after = noon + milliseconds(50);
   REQUIRE(text_at(system_clock::to_time_t(cron_prev(cex, just_after))) == "2026-06-01 12:00:00");

   // the duration of the argument is kept, and failure is reported the same
   // way as for cron_next
   auto const in_ms = time_point_cast<milliseconds>(noon);
   auto const out_ms = cron_prev(cex, in_ms);
   static_assert(std::is_same<decltype(out_ms), time_point<system_clock, milliseconds> const>::value,
                 "milliseconds in, milliseconds out");

   auto future = make_cron("0 15 10 * * ? 2050");
   REQUIRE(cron_prev(future, noon) == (system_clock::time_point::min)());
}

TEST_CASE("prev: an empty expression is refused", "[prev]")
{
   cronexpr cex;

   REQUIRE_THROWS_AS(cron_prev(cex, time_at("2026-06-01 12:00:00")), bad_cronexpr);
}
