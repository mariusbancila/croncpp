#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "croncpp.h"

#define ARE_EQUAL(x, y)       REQUIRE(x == y)
#define CRON_STD_EQUAL(x, y)  ARE_EQUAL(make_cron(x), make_cron(y))
#define CRON_ORCL_EQUAL(x, y) ARE_EQUAL(make_cron<cron::cron_oracle_traits>(x), make_cron<cron::cron_oracle_traits>(y))

using namespace cron;
/*
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
*/
TEST_CASE("Equivalent standard expressions", "")
{
   CRON_STD_EQUAL("* * * 2 * *", "* * * 2 * ?");
   CRON_STD_EQUAL("57,59 * * * * *", "57/2 * * * * *");
   CRON_STD_EQUAL("1,3,5 * * * * *", "1-6/2 * * * * *");
   CRON_STD_EQUAL("* * 4,8,12,16,20 * * *", "* * 4/4 * * *");
   CRON_STD_EQUAL("* * * * * 0-6", "* * * * * TUE,WED,THU,FRI,SAT,SUN,MON");
   CRON_STD_EQUAL("* * * * * 0", "* * * * * SUN");
   CRON_STD_EQUAL("* * * * 1-12 *", "* * * * FEB,JAN,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC *");
   CRON_STD_EQUAL("* * * * 2 *", "* * * * Feb *");
   CRON_STD_EQUAL("*  *  * *  1 *", "* * * * 1 *");
}

TEST_CASE("Equivalent Oracle expressions", "")
{
   CRON_ORCL_EQUAL("* * * 2 * *", "* * * 2 * ?");
   CRON_ORCL_EQUAL("57,59 * * * * *", "57/2 * * * * *");
   CRON_ORCL_EQUAL("1,3,5 * * * * *", "1-6/2 * * * * *");
   CRON_ORCL_EQUAL("* * 4,8,12,16,20 * * *", "* * 4/4 * * *");
   CRON_ORCL_EQUAL("* * * * * 1-7", "* * * * * TUE,WED,THU,FRI,SAT,SUN,MON");
   CRON_ORCL_EQUAL("* * * * * 7", "* * * * * SAT");
   CRON_ORCL_EQUAL("* * * * 0-11 *", "* * * * FEB,JAN,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC *");
   CRON_ORCL_EQUAL("* * * * 1 *", "* * * * Feb *");
   CRON_ORCL_EQUAL("*  *  * *  1 *", "* * * * 1 *");
}