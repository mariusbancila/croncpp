#include "catch.hpp"
#include "croncpp.h"

#include <string>
#include <vector>

#define CRON_EXPECT_EXCEPT(x)    REQUIRE_THROWS_AS(make_cron(x), bad_cronexpr)

using namespace cron;

namespace
{
   // A traits type as it would have been written before the year field
   // existed: no CRON_MIN_YEARS, so it keeps taking six fields.
   struct traits_without_years
   {
      static const cron_int CRON_MIN_SECONDS = 0;
      static const cron_int CRON_MAX_SECONDS = 59;

      static const cron_int CRON_MIN_MINUTES = 0;
      static const cron_int CRON_MAX_MINUTES = 59;

      static const cron_int CRON_MIN_HOURS = 0;
      static const cron_int CRON_MAX_HOURS = 23;

      static const cron_int CRON_MIN_DAYS_OF_WEEK = 0;
      static const cron_int CRON_MAX_DAYS_OF_WEEK = 6;

      static const cron_int CRON_MIN_DAYS_OF_MONTH = 1;
      static const cron_int CRON_MAX_DAYS_OF_MONTH = 31;

      static const cron_int CRON_MIN_MONTHS = 1;
      static const cron_int CRON_MAX_MONTHS = 12;

      static const cron_int CRON_MAX_YEARS_DIFF = 4;

#ifdef CRONCPP_IS_CPP17
      static const inline std::vector<std::string> DAYS = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
      static const inline std::vector<std::string> MONTHS = { "NIL", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
#else
      static std::vector<std::string>& DAYS()
      {
         static std::vector<std::string> days = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
         return days;
      }

      static std::vector<std::string>& MONTHS()
      {
         static std::vector<std::string> months = { "NIL", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
         return months;
      }
#endif
   };

   // A traits type that accepts a narrower range than croncpp stores.
   struct traits_with_narrow_years : traits_without_years
   {
      static const int CRON_MIN_YEARS = 2000;
      static const int CRON_MAX_YEARS = 2050;
   };

   template <typename Traits>
   std::time_t next_of(std::string const & expr, std::string const & from)
   {
      auto cex = make_cron<Traits>(expr);
      auto date = utils::to_tm(from);

      return cron_next<Traits>(cex, utils::tm_to_time(date));
   }

   template <typename Traits>
   std::string next_str(std::string const & expr, std::string const & from)
   {
      auto const result = next_of<Traits>(expr, from);
      if (INVALID_TIME == result) return "none";

      std::tm tm;
      if (utils::time_to_tm(&result, &tm) == nullptr) return "invalid";

      return utils::to_string(tm);
   }

   std::string next(std::string const & expr, std::string const & from)
   {
      return next_str<cron_standard_traits>(expr, from);
   }
}

TEST_CASE("years: the field is optional", "[years]")
{
   REQUIRE_NOTHROW(make_cron("0 15 10 * * ?"));
   REQUIRE_NOTHROW(make_cron("0 15 10 * * ? *"));
   REQUIRE_NOTHROW(make_cron("0 15 10 * * ? 2005"));

   // an unrestricted year field means the same as leaving it out
   REQUIRE(make_cron("0 15 10 * * ?") == make_cron("0 15 10 * * ? *"));

   CRON_EXPECT_EXCEPT("0 15 10 * * ? 2005 7");
   CRON_EXPECT_EXCEPT("0 15 10 * * ");
}

TEST_CASE("years: a single year", "[years]")
{
   REQUIRE(next("0 15 10 * * ? 2005", "2004-06-01 00:00:00") == "2005-01-01 10:15:00");
   REQUIRE(next("0 15 10 * * ? 2005", "2005-06-01 00:00:00") == "2005-06-01 10:15:00");

   // the year has gone by, so there is no next occurrence at all
   REQUIRE(next("0 15 10 * * ? 2005", "2020-06-01 00:00:00") == "none");
   REQUIRE(next_of<cron_standard_traits>("0 15 10 * * ? 2005", "2020-06-01 00:00:00") == INVALID_TIME);
}

TEST_CASE("years: ranges, lists and increments", "[years]")
{
   REQUIRE(next("0 15 10 * * ? 2002-2006", "2001-06-01 00:00:00") == "2002-01-01 10:15:00");
   REQUIRE(next("0 15 10 * * ? 2002-2006", "2006-12-31 23:00:00") == "none");
   REQUIRE(next("0 15 10 * * ? 2002,2010", "2003-01-01 00:00:00") == "2010-01-01 10:15:00");
   // 2000/5 is 2000, 2005, 2010 and so on, so a date inside 2005 matches it
   REQUIRE(next("0 15 10 * * ? 2000/5", "2001-01-01 00:00:00") == "2005-01-01 10:15:00");
   REQUIRE(next("0 15 10 * * ? 2000/5", "2005-06-01 00:00:00") == "2005-06-01 10:15:00");
   REQUIRE(next("0 15 10 * * ? 2000/5", "2006-06-01 00:00:00") == "2010-01-01 10:15:00");
}

TEST_CASE("years: a year further off than the search normally reaches", "[years]")
{
   // the search gives up after CRON_MAX_YEARS_DIFF years when no year is
   // named; naming one has to lift that
   REQUIRE(next("0 15 10 * * ? 2050", "2026-06-01 00:00:00") == "2050-01-01 10:15:00");
   REQUIRE(next("0 15 10 * * ? 2099", "1980-06-01 00:00:00") == "2099-01-01 10:15:00");
}

TEST_CASE("years: combined with the other fields", "[years]")
{
   // the leap day exists in 2024 but in none of 2021 to 2023
   REQUIRE(next("0 0 0 29 2 ? 2024", "2020-06-01 00:00:00") == "2024-02-29 00:00:00");
   REQUIRE(next("0 0 0 29 2 ? 2021-2023", "2020-06-01 00:00:00") == "none");

   // the last Friday of January 2002, written in quartz numbering
   REQUIRE(next_str<cron_quartz_traits>("0 15 10 ? * 6L 2002-2006", "2001-06-01 00:00:00")
           == "2002-01-25 10:15:00");

   REQUIRE(next("0 15 10 ? * 5#2 2005", "2004-06-01 00:00:00") == "2005-01-14 10:15:00");
   REQUIRE(next("0 15 10 L * ? 2005", "2004-06-01 00:00:00") == "2005-01-31 10:15:00");
}

TEST_CASE("years: outside the supported range", "[years]")
{
   CRON_EXPECT_EXCEPT("0 15 10 * * ? 1969");
   CRON_EXPECT_EXCEPT("0 15 10 * * ? 2100");
   CRON_EXPECT_EXCEPT("0 15 10 * * ? 1960-1980");
   CRON_EXPECT_EXCEPT("0 15 10 * * ? 2090-2110");
   CRON_EXPECT_EXCEPT("0 15 10 * * ? 0");
   CRON_EXPECT_EXCEPT("0 15 10 * * ? 20050");
   CRON_EXPECT_EXCEPT("0 15 10 * * ? YEAR");

   REQUIRE_NOTHROW(make_cron("0 15 10 * * ? 1970"));
   REQUIRE_NOTHROW(make_cron("0 15 10 * * ? 2099"));
}

TEST_CASE("years: expressions differing only by year are not equal", "[years]")
{
   REQUIRE(make_cron("0 15 10 * * ? 2005") != make_cron("0 15 10 * * ? 2006"));
   REQUIRE(make_cron("0 15 10 * * ? 2005") != make_cron("0 15 10 * * ?"));
   REQUIRE(make_cron("0 15 10 * * ? 2002-2003") == make_cron("0 15 10 * * ? 2002,2003"));
}

TEST_CASE("years: every traits type accepts the field", "[years]")
{
   REQUIRE(next_str<cron_standard_traits>("0 15 10 * * ? 2005", "2004-06-01 00:00:00") == "2005-01-01 10:15:00");
   REQUIRE(next_str<cron_quartz_traits>("0 15 10 * * ? 2005", "2004-06-01 00:00:00") == "2005-01-01 10:15:00");
   REQUIRE(next_str<cron_oracle_traits>("0 15 10 * * ? 2005", "2004-06-01 00:00:00") == "2005-01-01 10:15:00");
}

TEST_CASE("years: the traits opt in is detected", "[years]")
{
   static_assert(supports_years<cron_standard_traits>::value, "supplied traits support years");
   static_assert(supports_years<cron_quartz_traits>::value, "supplied traits support years");
   static_assert(supports_years<cron_oracle_traits>::value, "supplied traits support years");
   static_assert(!supports_years<traits_without_years>::value, "these do not declare a range");
   static_assert(supports_years<traits_with_narrow_years>::value, "these do declare one");

   REQUIRE(supports_years<cron_standard_traits>::value);
   REQUIRE(!supports_years<traits_without_years>::value);
   REQUIRE(supports_years<traits_with_narrow_years>::value);
}

TEST_CASE("years: traits that do not declare a year range", "[years]")
{
   // six fields go on working, exactly as before the field existed
   REQUIRE_NOTHROW(make_cron<traits_without_years>("0 15 10 * * ?"));
   REQUIRE_NOTHROW(make_cron<traits_without_years>("0 0 0 29 2 *"));
   REQUIRE(next_str<traits_without_years>("0 15 10 * * ?", "2004-06-01 00:00:00") == "2004-06-01 10:15:00");
   REQUIRE(next_str<traits_without_years>("0 15 10 ? * 5#2", "2011-04-30 23:30:00") == "2011-05-13 10:15:00");

   // a seventh is refused rather than silently ignored
   REQUIRE_THROWS_AS(make_cron<traits_without_years>("0 15 10 * * ? 2005"), bad_cronexpr);
   REQUIRE_THROWS_AS(make_cron<traits_without_years>("0 15 10 * * ? *"), bad_cronexpr);
   REQUIRE_THROWS_AS(make_cron<traits_without_years>("0 15 10 * * ? 2002-2006"), bad_cronexpr);
}

TEST_CASE("years: traits that declare a narrower range than croncpp stores", "[years]")
{
   // inside the range the traits allow
   REQUIRE_NOTHROW(make_cron<traits_with_narrow_years>("0 15 10 * * ? 2000"));
   REQUIRE_NOTHROW(make_cron<traits_with_narrow_years>("0 15 10 * * ? 2050"));
   REQUIRE_NOTHROW(make_cron<traits_with_narrow_years>("0 15 10 * * ? 2000-2050"));
   REQUIRE(next_str<traits_with_narrow_years>("0 15 10 * * ? 2005", "2004-06-01 00:00:00")
           == "2005-01-01 10:15:00");

   // outside it, even though croncpp itself could store those years
   REQUIRE_THROWS_AS(make_cron<traits_with_narrow_years>("0 15 10 * * ? 1999"), bad_cronexpr);
   REQUIRE_THROWS_AS(make_cron<traits_with_narrow_years>("0 15 10 * * ? 2051"), bad_cronexpr);
   REQUIRE_THROWS_AS(make_cron<traits_with_narrow_years>("0 15 10 * * ? 1990-2010"), bad_cronexpr);
   REQUIRE_THROWS_AS(make_cron<traits_with_narrow_years>("0 15 10 * * ? 2040-2060"), bad_cronexpr);
   REQUIRE_THROWS_AS(make_cron<traits_with_narrow_years>("0 15 10 * * ? 1999,2005"), bad_cronexpr);

   // the same years are fine for the supplied traits, so this is the traits
   // narrowing the range and not croncpp rejecting them outright
   REQUIRE_NOTHROW(make_cron<cron_standard_traits>("0 15 10 * * ? 1999"));
   REQUIRE_NOTHROW(make_cron<cron_standard_traits>("0 15 10 * * ? 2051"));

   // an unrestricted year field means every year the traits allow, not every
   // year croncpp is able to store
   REQUIRE_NOTHROW(make_cron<traits_with_narrow_years>("0 15 10 * * ? *"));
   REQUIRE(next_str<traits_with_narrow_years>("0 15 10 * * ? *", "2004-06-01 00:00:00")
           == "2004-06-01 10:15:00");
   REQUIRE(next_str<traits_with_narrow_years>("0 15 10 * * ? *", "2060-06-01 00:00:00") == "none");

   // and it is then the same as naming that whole range
   REQUIRE(make_cron<traits_with_narrow_years>("0 15 10 * * ? *") ==
           make_cron<traits_with_narrow_years>("0 15 10 * * ? 2000-2050"));

   // leaving the field out entirely is unrestricted in the same way
   REQUIRE(make_cron<traits_with_narrow_years>("0 15 10 * * ?") ==
           make_cron<traits_with_narrow_years>("0 15 10 * * ? *"));
}

TEST_CASE("years: to_cronstr keeps the original text", "[years]")
{
   REQUIRE(to_cronstr(make_cron("0 15 10 * * ? 2005")) == "0 15 10 * * ? 2005");
   REQUIRE(to_cronstr(make_cron("0 15 10 * * ? 2002-2006")) == "0 15 10 * * ? 2002-2006");
}
