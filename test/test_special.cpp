#include "catch.hpp"
#include "croncpp.h"

#include <string>

#define CRON_EXPECT_EXCEPT(x)    REQUIRE_THROWS_AS(make_cron(x), bad_cronexpr)

using namespace cron;

// The L, W and # special characters. The dates asserted below come from the
// calendar, not from croncpp:
//
//   2011-04-30 is a Saturday and the last day of April
//   May 2011 has Fridays on the 6th, 13th, 20th and 27th
//   July 2011 has five Fridays: the 1st, 8th, 15th, 22nd and 29th
//   2011-05-15 is a Sunday, 2011-10-15 a Saturday
//   2011-01-01 is a Saturday, 2011-07-31 a Sunday
//   February 2011 has 28 days, February 2012 has 29

namespace
{
   template <typename Traits = cron_standard_traits>
   std::string next_from(std::string const & expr, std::string const & from)
   {
      auto cex = make_cron<Traits>(expr);
      auto date = utils::to_tm(from);

      return utils::to_string(cron_next<Traits>(cex, date));
   }
}

TEST_CASE("special: # selects the nth weekday of the month", "[special]")
{
   // the expression from issue 18: the second Friday
   REQUIRE(next_from("0 30 23 ? * 5#2", "2011-04-30 23:30:00") == "2011-05-13 23:30:00");

   REQUIRE(next_from("0 30 23 ? * 5#1", "2011-04-30 23:30:00") == "2011-05-06 23:30:00");
   REQUIRE(next_from("0 30 23 ? * 5#3", "2011-04-30 23:30:00") == "2011-05-20 23:30:00");
   REQUIRE(next_from("0 30 23 ? * 5#4", "2011-04-30 23:30:00") == "2011-05-27 23:30:00");

   // May has only four Fridays, so a fifth Friday is next found in July
   REQUIRE(next_from("0 30 23 ? * 5#5", "2011-04-30 23:30:00") == "2011-07-29 23:30:00");

   // consecutive occurrences are a month apart, not a week
   REQUIRE(next_from("0 30 23 ? * 5#2", "2011-05-13 23:30:00") == "2011-06-10 23:30:00");

   // written with a name rather than a number
   REQUIRE(next_from("0 30 23 ? * FRI#2", "2011-04-30 23:30:00") == "2011-05-13 23:30:00");
}

TEST_CASE("special: # follows the numbering of the traits", "[special]")
{
   // standard counts Sunday as 0, quartz and oracle count it as 1
   auto const expected = "2011-05-13 23:30:00";

   REQUIRE(next_from<cron_standard_traits>("0 30 23 ? * 5#2", "2011-04-30 23:30:00") == expected);
   REQUIRE(next_from<cron_quartz_traits>("0 30 23 ? * 6#2", "2011-04-30 23:30:00") == expected);
   REQUIRE(next_from<cron_oracle_traits>("0 30 23 ? * 6#2", "2011-04-30 23:30:00") == expected);
}

TEST_CASE("special: L after a weekday selects the last one of the month", "[special]")
{
   REQUIRE(next_from("0 30 23 ? * 5L", "2011-04-30 23:30:00") == "2011-05-27 23:30:00");
   REQUIRE(next_from("0 30 23 ? * FRIL", "2011-04-30 23:30:00") == "2011-05-27 23:30:00");
   REQUIRE(next_from("0 30 23 ? * 5L", "2011-05-27 23:30:00") == "2011-06-24 23:30:00");

   // in July the last Friday is also the fifth
   REQUIRE(next_from("0 30 23 ? * 5L", "2011-06-30 23:30:00") == "2011-07-29 23:30:00");
}

TEST_CASE("special: L on its own in the day of week field means Saturday", "[special]")
{
   // as in Quartz, where L alone in that field is 7, meaning SAT
   REQUIRE(next_from("0 30 23 ? * L", "2011-04-30 23:30:00") == "2011-05-07 23:30:00");
   REQUIRE(next_from("0 30 23 ? * L", "2011-05-07 23:30:00") == "2011-05-14 23:30:00");
}

TEST_CASE("special: L in the day of month field means the last day", "[special]")
{
   REQUIRE(next_from("0 30 23 L * ?", "2011-04-30 23:30:00") == "2011-05-31 23:30:00");
   REQUIRE(next_from("0 30 23 L * ?", "2011-01-31 23:30:00") == "2011-02-28 23:30:00");
   REQUIRE(next_from("0 30 23 L * ?", "2012-01-31 23:30:00") == "2012-02-29 23:30:00");
   REQUIRE(next_from("0 30 23 L 2 ?", "2011-03-01 00:00:00") == "2012-02-29 23:30:00");
}

TEST_CASE("special: W moves to the nearest weekday", "[special]")
{
   // the 15th of May 2011 is a Sunday, so it moves forward to Monday
   REQUIRE(next_from("0 30 23 15W * ?", "2011-04-30 23:30:00") == "2011-05-16 23:30:00");

   // the 15th of October 2011 is a Saturday, so it moves back to Friday
   REQUIRE(next_from("0 30 23 15W * ?", "2011-09-30 23:30:00") == "2011-10-14 23:30:00");

   // a weekday is left alone
   REQUIRE(next_from("0 30 23 15W * ?", "2011-05-31 23:30:00") == "2011-06-15 23:30:00");
}

TEST_CASE("special: W does not leave the month", "[special]")
{
   // the 1st of January 2011 is a Saturday: stepping back would leave the
   // month, so it moves forward to Monday the 3rd
   REQUIRE(next_from("0 30 23 1W * ?", "2010-12-15 23:30:00") == "2011-01-03 23:30:00");

   // the 31st of July 2011 is a Sunday: stepping forward would leave the
   // month, so it moves back to Friday the 29th
   REQUIRE(next_from("0 30 23 31W * ?", "2011-07-01 00:00:00") == "2011-07-29 23:30:00");
}

TEST_CASE("special: LW is the last weekday of the month", "[special]")
{
   // April 2011 ends on Saturday the 30th, so the last weekday is Friday
   REQUIRE(next_from("0 30 23 LW * ?", "2011-04-01 00:00:00") == "2011-04-29 23:30:00");

   // May 2011 ends on a Tuesday
   REQUIRE(next_from("0 30 23 LW * ?", "2011-05-01 00:00:00") == "2011-05-31 23:30:00");

   // December 2011 ends on Saturday the 31st
   REQUIRE(next_from("0 30 23 LW * ?", "2011-12-01 00:00:00") == "2011-12-30 23:30:00");
}

TEST_CASE("special: qualified expressions are not equal to unqualified ones", "[special]")
{
   REQUIRE(make_cron("0 30 23 ? * 5#2") != make_cron("0 30 23 ? * 5#3"));
   REQUIRE(make_cron("0 30 23 ? * 5#2") != make_cron("0 30 23 ? * 5"));
   REQUIRE(make_cron("0 30 23 ? * 5L") != make_cron("0 30 23 ? * 5"));
   REQUIRE(make_cron("0 30 23 ? * 5L") != make_cron("0 30 23 ? * 5#5"));
   REQUIRE(make_cron("0 30 23 L * ?") != make_cron("0 30 23 1 * ?"));
   REQUIRE(make_cron("0 30 23 15W * ?") != make_cron("0 30 23 15 * ?"));
   REQUIRE(make_cron("0 30 23 LW * ?") != make_cron("0 30 23 L * ?"));

   REQUIRE(make_cron("0 30 23 ? * 5#2") == make_cron("0 30 23 ? * FRI#2"));
   REQUIRE(make_cron("0 30 23 ? * 5L") == make_cron("0 30 23 ? * FRIL"));
}

TEST_CASE("special: the qualifiers apply to a single value", "[special]")
{
   CRON_EXPECT_EXCEPT("0 30 23 ? * 5#2,6#3");
   CRON_EXPECT_EXCEPT("0 30 23 ? * 5-6#2");
   CRON_EXPECT_EXCEPT("0 30 23 ? * 5L,6L");
   CRON_EXPECT_EXCEPT("0 30 23 ? * 5-6L");
   CRON_EXPECT_EXCEPT("0 30 23 1,15W * ?");
   CRON_EXPECT_EXCEPT("0 30 23 1-15W * ?");
   CRON_EXPECT_EXCEPT("0 30 23 L,1 * ?");
}

TEST_CASE("special: malformed qualifiers are rejected", "[special]")
{
   CRON_EXPECT_EXCEPT("0 30 23 ? * 5#0");   // there is no zeroth Friday
   CRON_EXPECT_EXCEPT("0 30 23 ? * 5#6");   // nor a sixth
   CRON_EXPECT_EXCEPT("0 30 23 ? * 5#");
   CRON_EXPECT_EXCEPT("0 30 23 ? * #2");
   CRON_EXPECT_EXCEPT("0 30 23 ? * 5#2#3");
   CRON_EXPECT_EXCEPT("0 30 23 ? * 8#2");   // no such weekday
   CRON_EXPECT_EXCEPT("0 30 23 W * ?");
   CRON_EXPECT_EXCEPT("0 30 23 0W * ?");
   CRON_EXPECT_EXCEPT("0 30 23 32W * ?");
   CRON_EXPECT_EXCEPT("0 30 23 WL * ?");
   CRON_EXPECT_EXCEPT("0 30 23 15L * ?");

   // still not allowed in any other field
   CRON_EXPECT_EXCEPT("0 30 23L * * ?");
   CRON_EXPECT_EXCEPT("0 30W 23 * * ?");
   CRON_EXPECT_EXCEPT("0 30 23 * L ?");
   CRON_EXPECT_EXCEPT("0 30 23 * 5#2 ?");
}

TEST_CASE("special: L and W against dates that never occur", "[special]")
{
   // the last day of February always exists
   REQUIRE_NOTHROW(make_cron("0 0 0 L 2 *"));
   REQUIRE_NOTHROW(make_cron("0 0 0 LW 2 *"));

   // but there is no 31st of February to move away from
   CRON_EXPECT_EXCEPT("0 0 0 31W 2 *");
   REQUIRE_NOTHROW(make_cron("0 0 0 31W 1 *"));
}

TEST_CASE("special: to_cronstr keeps the original text", "[special]")
{
   REQUIRE(to_cronstr(make_cron("0 30 23 ? * 5#2")) == "0 30 23 ? * 5#2");
   REQUIRE(to_cronstr(make_cron("0 30 23 LW * ?")) == "0 30 23 LW * ?");
   REQUIRE(to_cronstr(make_cron("0 30 23 15W * ?")) == "0 30 23 15W * ?");
}
