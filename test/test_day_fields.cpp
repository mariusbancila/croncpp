#include "catch.hpp"
#include "croncpp.h"

#include <string>
#include <vector>

using namespace cron;

// How the day of month and day of week fields combine when an expression
// restricts both of them. POSIX cron matches a date against either field,
// Quartz refuses the expression and wants ? in one of them, and a traits type
// that says nothing keeps intersecting the two.

namespace
{
   struct traits_without_rule
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

   struct traits_rejecting : traits_without_rule
   {
      static const day_field_rule CRON_DAY_FIELD_RULE = day_field_rule::reject;
   };

   struct traits_either : traits_without_rule
   {
      static const day_field_rule CRON_DAY_FIELD_RULE = day_field_rule::either;
   };

   template <typename Traits>
   std::string next_str(std::string const & expr, std::string const & from)
   {
      auto cex = make_cron<Traits>(expr);
      auto date = utils::to_tm(from);

      return utils::to_string(cron_next<Traits>(cex, date));
   }

   std::string next(std::string const & expr, std::string const & from)
   {
      return next_str<cron_standard_traits>(expr, from);
   }
}

TEST_CASE("day fields: issue 12, either field may match under POSIX rules", "[days]")
{
   // the 1st of January 2021 is a Friday, and it is the 1st, so it matches the
   // day of month half of "0 0 0 1 1 1"
   REQUIRE(next("0 0 0 1 1 1", "2020-10-08 00:00:00") == "2021-01-01 00:00:00");

   // and the Mondays of January match the other half
   REQUIRE(next("0 0 0 1 1 1", "2021-01-01 00:00:00") == "2021-01-04 00:00:00");
   REQUIRE(next("0 0 0 1 1 1", "2021-01-04 00:00:00") == "2021-01-11 00:00:00");

   // the 15th of any month, or any Monday
   REQUIRE(next("0 0 0 15 * 1", "2021-02-28 00:00:00") == "2021-03-01 00:00:00"); // a Monday
   REQUIRE(next("0 0 0 15 * 1", "2021-03-01 00:00:00") == "2021-03-08 00:00:00"); // a Monday
   REQUIRE(next("0 0 0 15 * 1", "2021-03-09 00:00:00") == "2021-03-15 00:00:00"); // both
   REQUIRE(next("0 0 0 15 * 1", "2021-04-13 00:00:00") == "2021-04-15 00:00:00"); // the 15th, a Thursday
}

TEST_CASE("day fields: a field is restricted unless it is * or ?", "[days]")
{
   // 1-31 covers every day but is still a restriction, so the rule applies and
   // the expression matches every day of January rather than only Mondays
   REQUIRE(next("0 0 0 1-31 1 1", "2020-12-31 00:00:00") == "2021-01-01 00:00:00");
   REQUIRE(next("0 0 0 1-31 1 1", "2021-01-01 00:00:00") == "2021-01-02 00:00:00");

   // with the day of month left open, only Mondays match
   REQUIRE(next("0 0 0 * 1 1", "2020-12-31 00:00:00") == "2021-01-04 00:00:00");

   // so the two are not the same expression
   REQUIRE(make_cron("0 0 0 1-31 1 1") != make_cron("0 0 0 * 1 1"));
}

TEST_CASE("day fields: one field open behaves as before", "[days]")
{
   REQUIRE(next("0 0 0 1 1 ?", "2020-10-08 00:00:00") == "2021-01-01 00:00:00");
   REQUIRE(next("0 0 0 ? 1 1", "2020-10-08 00:00:00") == "2021-01-04 00:00:00");
   REQUIRE(next("0 0 0 1 1 *", "2020-10-08 00:00:00") == "2021-01-01 00:00:00");
   REQUIRE(next("0 0 0 * 1 *", "2020-12-31 00:00:00") == "2021-01-01 00:00:00");

   // ? and * mean the same thing in either field
   REQUIRE(make_cron("0 0 0 1 1 ?") == make_cron("0 0 0 1 1 *"));
   REQUIRE(make_cron("0 0 0 ? 1 1") == make_cron("0 0 0 * 1 1"));
}

TEST_CASE("day fields: quartz and oracle refuse to guess", "[days]")
{
   REQUIRE_THROWS_AS(make_cron<cron_quartz_traits>("0 0 0 1 1 1"), bad_cronexpr);
   REQUIRE_THROWS_AS(make_cron<cron_oracle_traits>("0 0 0 1 1 1"), bad_cronexpr);
   REQUIRE_THROWS_AS(make_cron<cron_quartz_traits>("0 0 0 1-15 * 2-6"), bad_cronexpr);

   // a day of month qualified with L or W is a restriction too
   REQUIRE_THROWS_AS(make_cron<cron_quartz_traits>("0 0 0 L * 6"), bad_cronexpr);
   REQUIRE_THROWS_AS(make_cron<cron_quartz_traits>("0 0 0 15W * 6"), bad_cronexpr);

   // leaving one of them open is what quartz asks for
   REQUIRE_NOTHROW(make_cron<cron_quartz_traits>("0 0 0 1 1 ?"));
   REQUIRE_NOTHROW(make_cron<cron_quartz_traits>("0 0 0 ? 1 1"));
   REQUIRE_NOTHROW(make_cron<cron_quartz_traits>("0 0 0 * * *"));
   REQUIRE_NOTHROW(make_cron<cron_quartz_traits>("0 0 0 L * ?"));
   REQUIRE_NOTHROW(make_cron<cron_quartz_traits>("0 0 0 ? * 6#2"));
   REQUIRE_NOTHROW(make_cron<cron_oracle_traits>("0 0 0 ? 1 1"));
}

TEST_CASE("day fields: the rule comes from the traits", "[days]")
{
   static_assert(day_rule<cron_standard_traits>::value == day_field_rule::either,
                 "the standard traits follow POSIX");
   static_assert(day_rule<cron_quartz_traits>::value == day_field_rule::reject,
                 "quartz wants ? in one field");
   static_assert(day_rule<cron_oracle_traits>::value == day_field_rule::reject,
                 "so does the oracle format");
   static_assert(day_rule<traits_without_rule>::value == day_field_rule::intersect,
                 "traits saying nothing keep intersecting");

   REQUIRE(day_rule<traits_either>::value == day_field_rule::either);
   REQUIRE(day_rule<traits_rejecting>::value == day_field_rule::reject);
}

TEST_CASE("day fields: traits that state no rule are unaffected", "[days]")
{
   // both fields have to match, which is what croncpp did before the rule
   // existed: the first of January that is also a Monday
   REQUIRE(next_str<traits_without_rule>("0 0 0 1 1 1", "2020-10-08 00:00:00")
           == "2024-01-01 00:00:00");

   // the same expression under each of the other two rules
   REQUIRE(next_str<traits_either>("0 0 0 1 1 1", "2020-10-08 00:00:00")
           == "2021-01-01 00:00:00");
   REQUIRE_THROWS_AS(make_cron<traits_rejecting>("0 0 0 1 1 1"), bad_cronexpr);

   // and an expression leaving one field open means the same under all three
   REQUIRE(next_str<traits_without_rule>("0 0 0 1 1 ?", "2020-10-08 00:00:00")
           == "2021-01-01 00:00:00");
   REQUIRE(next_str<traits_either>("0 0 0 1 1 ?", "2020-10-08 00:00:00")
           == "2021-01-01 00:00:00");
   REQUIRE(next_str<traits_rejecting>("0 0 0 1 1 ?", "2020-10-08 00:00:00")
           == "2021-01-01 00:00:00");
}
