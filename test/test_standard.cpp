#include "catch.hpp"
#include "croncpp.h"

#define ARE_EQUAL(x, y)          REQUIRE(x == y)
#define CRON_EXPR(x)             make_cron(x)
#define CRON_STD_EQUAL(x, y)     ARE_EQUAL(make_cron(x), make_cron(y))
#define CRON_EXPECT_EXCEPT(x)    REQUIRE_THROWS_AS(make_cron(x), bad_cronexpr)
#define CRON_EXPECT_MSG(x, msg)  REQUIRE_THROWS_WITH(make_cron(x), msg)

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

TEST_CASE("standard: check seconds", "[std]")
{
   CRON_STD_EQUAL("*/5 * * * * *", "0,5,10,15,20,25,30,35,40,45,50,55 * * * * *");
   CRON_STD_EQUAL("1/6 * * * * *", "1,7,13,19,25,31,37,43,49,55 * * * * *");
   CRON_STD_EQUAL("0/30 * * * * *", "0,30 * * * * *");
   CRON_STD_EQUAL("0-5 * * * * *", "0,1,2,3,4,5 * * * * *");
   CRON_STD_EQUAL("55/1 * * * * *", "55,56,57,58,59 * * * * *");
   CRON_STD_EQUAL("57,59 * * * * *", "57/2 * * * * *");
   CRON_STD_EQUAL("1,3,5 * * * * *", "1-6/2 * * * * *");
   CRON_STD_EQUAL("0,5,10,15 * * * * *", "0-15/5 * * * * *");
}

TEST_CASE("standard: check minutes", "[std]")
{
   CRON_STD_EQUAL("* */5 * * * *", "* 0,5,10,15,20,25,30,35,40,45,50,55 * * * *");
   CRON_STD_EQUAL("* 1/6 * * * *", "* 1,7,13,19,25,31,37,43,49,55 * * * *");
   CRON_STD_EQUAL("* 0/30 * * * *", "* 0,30 * * * *");
   CRON_STD_EQUAL("* 0-5 * * * *", "* 0,1,2,3,4,5 * * * *");
   CRON_STD_EQUAL("* 55/1 * * * *", "* 55,56,57,58,59 * * * *");
   CRON_STD_EQUAL("* 57,59 * * * *", "* 57/2 * * * *");
   CRON_STD_EQUAL("* 1,3,5 * * * *", "* 1-6/2 * * * *");
   CRON_STD_EQUAL("* 0,5,10,15 * * * *", "* 0-15/5 * * * *");
}

TEST_CASE("standard: check hours", "[std]")
{
   CRON_STD_EQUAL("* * */5 * * *", "* * 0,5,10,15,20 * * *");
   CRON_STD_EQUAL("* * 1/6 * * *", "* * 1,7,13,19 * * *");
   CRON_STD_EQUAL("* * 0/12 * * *", "* * 0,12 * * *");
   CRON_STD_EQUAL("* * 0-5 * * *", "* * 0,1,2,3,4,5 * * *");
   CRON_STD_EQUAL("* * 15/1 * * *", "* * 15,16,17,18,19,20,21,22,23 * * *");
   CRON_STD_EQUAL("* * 17,19,21,23 * * *", "* * 17/2 * * *");
   CRON_STD_EQUAL("* * 1,3,5 * * *", "* * 1-6/2 * * *");
   CRON_STD_EQUAL("* * 0,5,10,15 * * *", "* * 0-15/5 * * *");
}

TEST_CASE("standard: check days of month", "[std]")
{
   CRON_STD_EQUAL("* * * 1-31 * *", "* * * 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 * *");
   CRON_STD_EQUAL("* * * 1/5 * *", "* * * 1,6,11,16,21,26,31 * *");
   CRON_STD_EQUAL("* * * 1,11,21,31 * *", "* * * 1-31/10 * *");
   CRON_STD_EQUAL("* * * */5 * *", "* * * 1,6,11,16,21,26,31 * *");
}

TEST_CASE("standard: check months", "[std]")
{
   CRON_STD_EQUAL("* * * * 1,6,11 *", "* * * * 1/5 *");
   CRON_STD_EQUAL("* * * * 1-12 *", "* * * * 1,2,3,4,5,6,7,8,9,10,11,12 *");
   CRON_STD_EQUAL("* * * * 1-12/3 *", "* * * * 1,4,7,10 *");
   CRON_STD_EQUAL("* * * * */2 *", "* * * * 1,3,5,7,9,11 *");
   CRON_STD_EQUAL("* * * * 2/2 *", "* * * * 2,4,6,8,10,12 *");
   CRON_STD_EQUAL("* * * * 1 *", "* * * * JAN *");
   CRON_STD_EQUAL("* * * * 2 *", "* * * * FEB *");
   CRON_STD_EQUAL("* * * * 3 *", "* * * * mar *");
   CRON_STD_EQUAL("* * * * 4 *", "* * * * apr *");
   CRON_STD_EQUAL("* * * * 5 *", "* * * * may *");
   CRON_STD_EQUAL("* * * * 6 *", "* * * * Jun *");
   CRON_STD_EQUAL("* * * * 7 *", "* * * * Jul *");
   CRON_STD_EQUAL("* * * * 8 *", "* * * * auG *");
   CRON_STD_EQUAL("* * * * 9 *", "* * * * sEp *");
   CRON_STD_EQUAL("* * * * 10 *", "* * * * oCT *");
   CRON_STD_EQUAL("* * * * 11 *", "* * * * nOV *");
   CRON_STD_EQUAL("* * * * 12 *", "* * * * DEC *");
   CRON_STD_EQUAL("* * * * 1,FEB *", "* * * * JAN,2 *");
   CRON_STD_EQUAL("* * * * 1,6,11 *", "* * * * NOV,JUN,jan *");
   CRON_STD_EQUAL("* * * * 1,6,11 *", "* * * * jan,jun,nov *");
   CRON_STD_EQUAL("* * * * 1,6,11 *", "* * * * jun,jan,nov *");
   CRON_STD_EQUAL("* * * * JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC *", "* * * * 1,2,3,4,5,6,7,8,9,10,11,12 *");
   CRON_STD_EQUAL("* * * * JUL,AUG,SEP,OCT,NOV,DEC,JAN,FEB,MAR,APR,MAY,JUN *", "* * * * 1,2,3,4,5,6,7,8,9,10,11,12 *");
}

TEST_CASE("standard: check days of week", "[std]")
{
   CRON_STD_EQUAL("* * * * * 0-6", "* * * * * 0,1,2,3,4,5,6");
   CRON_STD_EQUAL("* * * * * 0-6/2", "* * * * * 0,2,4,6");
   CRON_STD_EQUAL("* * * * * 0-6/3", "* * * * * 0,3,6");
   CRON_STD_EQUAL("* * * * * */3", "* * * * * 0,3,6");
   CRON_STD_EQUAL("* * * * * 1/3", "* * * * * 1,4");
   CRON_STD_EQUAL("* * * * * 0", "* * * * * SUN");
   CRON_STD_EQUAL("* * * * * 1", "* * * * * MON");
   CRON_STD_EQUAL("* * * * * 2", "* * * * * TUE");
   CRON_STD_EQUAL("* * * * * 3", "* * * * * WED");
   CRON_STD_EQUAL("* * * * * 4", "* * * * * THU");
   CRON_STD_EQUAL("* * * * * 5", "* * * * * FRI");
   CRON_STD_EQUAL("* * * * * 6", "* * * * * SAT");
   CRON_STD_EQUAL("* * * * * 0-6", "* * * * * SUN,MON,TUE,WED,THU,FRI,SAT");
   CRON_STD_EQUAL("* * * * * 0-6/2", "* * * * * SUN,TUE,THU,SAT");
   CRON_STD_EQUAL("* * * * * 0-6/3", "* * * * * SUN,WED,SAT");
   CRON_STD_EQUAL("* * * * * */3", "* * * * * SUN,WED,SAT");
   CRON_STD_EQUAL("* * * * * 1/3", "* * * * * MON,THU");
}

TEST_CASE("standard: invalid seconds", "[std]")
{
   CRON_EXPECT_EXCEPT("TEN * * * * *");
   CRON_EXPECT_EXCEPT("60 * * * * *");
   CRON_EXPECT_EXCEPT("-1 * * * * *");
   CRON_EXPECT_EXCEPT("0-60 * * * * *");
   CRON_EXPECT_EXCEPT("0-10-20 * * * * *");
   CRON_EXPECT_EXCEPT(", * * * * *");
   CRON_EXPECT_EXCEPT("0,,1 * * * * *");
   CRON_EXPECT_EXCEPT("0,1, * * * * *");
   CRON_EXPECT_EXCEPT(",0,1 * * * * *");
   CRON_EXPECT_EXCEPT("0/10/2 * * * * *");
   CRON_EXPECT_EXCEPT("/ * * * * *");
   CRON_EXPECT_EXCEPT("/2 * * * * *");
   CRON_EXPECT_EXCEPT("2/ * * * * *");
   CRON_EXPECT_EXCEPT("*/ * * * * *");
   CRON_EXPECT_EXCEPT("/* * * * * *");
   CRON_EXPECT_EXCEPT("-/ * * * * *");
   CRON_EXPECT_EXCEPT("/- * * * * *");
   CRON_EXPECT_EXCEPT("*-/ * * * * *");
   CRON_EXPECT_EXCEPT("-*/ * * * * *");
   CRON_EXPECT_EXCEPT("/-* * * * * *");
   CRON_EXPECT_EXCEPT("/*- * * * * *");
   CRON_EXPECT_EXCEPT("*2/ * * * * *");
   CRON_EXPECT_EXCEPT("/2* * * * * *");
   CRON_EXPECT_EXCEPT("-2/ * * * * *");
   CRON_EXPECT_EXCEPT("/2- * * * * *");
   CRON_EXPECT_EXCEPT("*2-/ * * * * *");
   CRON_EXPECT_EXCEPT("-2*/ * * * * *");
   CRON_EXPECT_EXCEPT("/2-* * * * * *");
   CRON_EXPECT_EXCEPT("/2*- * * * * *");
}

TEST_CASE("standard: invalid minutes", "[std]")
{
   CRON_EXPECT_EXCEPT("* TEN * * * *");
   CRON_EXPECT_EXCEPT("* 60 * * * *");
   CRON_EXPECT_EXCEPT("* -1 * * * *");
   CRON_EXPECT_EXCEPT("* 0-60 * * * *");
   CRON_EXPECT_EXCEPT("* 0-10-20 * * * *");
   CRON_EXPECT_EXCEPT("* , * * * *");
   CRON_EXPECT_EXCEPT("* 0,,1 * * * *");
   CRON_EXPECT_EXCEPT("* 0,1, * * * *");
   CRON_EXPECT_EXCEPT("* ,0,1 * * * *");
   CRON_EXPECT_EXCEPT("* 0/10/2 * * * *");
   CRON_EXPECT_EXCEPT("* / * * * *");
   CRON_EXPECT_EXCEPT("* /2 * * * *");
   CRON_EXPECT_EXCEPT("* 2/ * * * *");
   CRON_EXPECT_EXCEPT("* */ * * * *");
   CRON_EXPECT_EXCEPT("* /* * * * *");
   CRON_EXPECT_EXCEPT("* -/ * * * *");
   CRON_EXPECT_EXCEPT("* /- * * * *");
   CRON_EXPECT_EXCEPT("* *-/ * * * *");
   CRON_EXPECT_EXCEPT("* -*/ * * * *");
   CRON_EXPECT_EXCEPT("* /-* * * * *");
   CRON_EXPECT_EXCEPT("* /*- * * * *");
   CRON_EXPECT_EXCEPT("* *2/ * * * *");
   CRON_EXPECT_EXCEPT("* /2* * * * *");
   CRON_EXPECT_EXCEPT("* -2/ * * * *");
   CRON_EXPECT_EXCEPT("* /2- * * * *");
   CRON_EXPECT_EXCEPT("* *2-/ * * * *");
   CRON_EXPECT_EXCEPT("* -2*/ * * * *");
   CRON_EXPECT_EXCEPT("* /2-* * * * *");
   CRON_EXPECT_EXCEPT("* /2*- * * * *");
}

TEST_CASE("standard: invalid hours", "[std]")
{
   CRON_EXPECT_EXCEPT("* * TEN * * *");
   CRON_EXPECT_EXCEPT("* * 24 * * *");
   CRON_EXPECT_EXCEPT("* * -1 * * *");
   CRON_EXPECT_EXCEPT("* * 0-24 * * *");
   CRON_EXPECT_EXCEPT("* * 0-10-20 * * *");
   CRON_EXPECT_EXCEPT("* * , * * *");
   CRON_EXPECT_EXCEPT("* * 0,,1 * * *");
   CRON_EXPECT_EXCEPT("* * 0,1, * * *");
   CRON_EXPECT_EXCEPT("* * ,0,1 * * *");
   CRON_EXPECT_EXCEPT("* * 0/10/2 * * *");
   CRON_EXPECT_EXCEPT("* * / * * *");
   CRON_EXPECT_EXCEPT("* * /2 * * *");
   CRON_EXPECT_EXCEPT("* * 2/ * * *");
   CRON_EXPECT_EXCEPT("* * */ * * *");
   CRON_EXPECT_EXCEPT("* * /* * * *");
   CRON_EXPECT_EXCEPT("* * -/ * * *");
   CRON_EXPECT_EXCEPT("* * /- * * *");
   CRON_EXPECT_EXCEPT("* * *-/ * * *");
   CRON_EXPECT_EXCEPT("* * -*/ * * *");
   CRON_EXPECT_EXCEPT("* * /-* * * *");
   CRON_EXPECT_EXCEPT("* * /*- * * *");
   CRON_EXPECT_EXCEPT("* * *2/ * * *");
   CRON_EXPECT_EXCEPT("* * /2* * * *");
   CRON_EXPECT_EXCEPT("* * -2/ * * *");
   CRON_EXPECT_EXCEPT("* * /2- * * *");
   CRON_EXPECT_EXCEPT("* * *2-/ * * *");
   CRON_EXPECT_EXCEPT("* * -2*/ * * *");
   CRON_EXPECT_EXCEPT("* * /2-* * * *");
   CRON_EXPECT_EXCEPT("* * /2*- * * *");
}

TEST_CASE("standard: invalid days of month", "[std]")
{
   CRON_EXPECT_EXCEPT("* * * TEN * *");
   CRON_EXPECT_EXCEPT("* * * 32 * *");
   CRON_EXPECT_EXCEPT("* * * 0 * *");
   CRON_EXPECT_EXCEPT("* * * 0-32 * *");
   CRON_EXPECT_EXCEPT("* * * 0-10-20 * *");
   CRON_EXPECT_EXCEPT("* * * , * *");
   CRON_EXPECT_EXCEPT("* * * 0,,1 * *");
   CRON_EXPECT_EXCEPT("* * * 0,1, * *");
   CRON_EXPECT_EXCEPT("* * * ,0,1 * *");
   CRON_EXPECT_EXCEPT("* * * 0/10/2 * * *");
   CRON_EXPECT_EXCEPT("* * * / * *");
   CRON_EXPECT_EXCEPT("* * * /2 * *");
   CRON_EXPECT_EXCEPT("* * * 2/ * *");
   CRON_EXPECT_EXCEPT("* * * */ * *");
   CRON_EXPECT_EXCEPT("* * * /* * *");
   CRON_EXPECT_EXCEPT("* * * -/ * *");
   CRON_EXPECT_EXCEPT("* * * /- * *");
   CRON_EXPECT_EXCEPT("* * * *-/ * *");
   CRON_EXPECT_EXCEPT("* * * -*/ * *");
   CRON_EXPECT_EXCEPT("* * * /-* * *");
   CRON_EXPECT_EXCEPT("* * * /*- * *");
   CRON_EXPECT_EXCEPT("* * * *2/ * *");
   CRON_EXPECT_EXCEPT("* * * /2* * *");
   CRON_EXPECT_EXCEPT("* * * -2/ * *");
   CRON_EXPECT_EXCEPT("* * * /2- * *");
   CRON_EXPECT_EXCEPT("* * * *2-/ * *");
   CRON_EXPECT_EXCEPT("* * * -2*/ * *");
   CRON_EXPECT_EXCEPT("* * * /2-* * *");
   CRON_EXPECT_EXCEPT("* * * /2*- * *");
}

TEST_CASE("standard: invalid months", "[std]")
{
   CRON_EXPECT_EXCEPT("* * * * TEN *");
   CRON_EXPECT_EXCEPT("* * * * 13 *");
   CRON_EXPECT_EXCEPT("* * * * 0 *");
   CRON_EXPECT_EXCEPT("* * * * 0-13 *");
   CRON_EXPECT_EXCEPT("* * * * 0-10-11 *");
   CRON_EXPECT_EXCEPT("* * * * , *");
   CRON_EXPECT_EXCEPT("* * * * 0,,1 *");
   CRON_EXPECT_EXCEPT("* * * * 0,1, *");
   CRON_EXPECT_EXCEPT("* * * * ,0,1 *");
   CRON_EXPECT_EXCEPT("* * * * 0/10/2 *");
   CRON_EXPECT_EXCEPT("* * * * / *");
   CRON_EXPECT_EXCEPT("* * * * /2 *");
   CRON_EXPECT_EXCEPT("* * * * 2/ *");
   CRON_EXPECT_EXCEPT("* * * * */ *");
   CRON_EXPECT_EXCEPT("* * * * /* *");
   CRON_EXPECT_EXCEPT("* * * * -/ *");
   CRON_EXPECT_EXCEPT("* * * * /- *");
   CRON_EXPECT_EXCEPT("* * * * *-/ *");
   CRON_EXPECT_EXCEPT("* * * * -*/ *");
   CRON_EXPECT_EXCEPT("* * * * /-* *");
   CRON_EXPECT_EXCEPT("* * * * /*- *");
   CRON_EXPECT_EXCEPT("* * * * *2/ *");
   CRON_EXPECT_EXCEPT("* * * * /2* *");
   CRON_EXPECT_EXCEPT("* * * * -2/ *");
   CRON_EXPECT_EXCEPT("* * * * /2- *");
   CRON_EXPECT_EXCEPT("* * * * *2-/ *");
   CRON_EXPECT_EXCEPT("* * * * -2*/ *");
   CRON_EXPECT_EXCEPT("* * * * /2-* *");
   CRON_EXPECT_EXCEPT("* * * * /2*- *");
}

TEST_CASE("standard: invalid days of week", "[std]")
{
   CRON_EXPECT_EXCEPT("* * * * * TEN");
   CRON_EXPECT_EXCEPT("* * * * * 7");
   CRON_EXPECT_EXCEPT("* * * * * -1");
   CRON_EXPECT_EXCEPT("* * * * * -1-7");
   CRON_EXPECT_EXCEPT("* * * * * 0-5-6");
   CRON_EXPECT_EXCEPT("* * * * * ,");
   CRON_EXPECT_EXCEPT("* * * * * 0,,1");
   CRON_EXPECT_EXCEPT("* * * * * 0,1,");
   CRON_EXPECT_EXCEPT("* * * * * ,0,1");
   CRON_EXPECT_EXCEPT("* * * * * 0/2/2 *");
   CRON_EXPECT_EXCEPT("* * * * * /");
   CRON_EXPECT_EXCEPT("* * * * * /2");
   CRON_EXPECT_EXCEPT("* * * * * 2/");
   CRON_EXPECT_EXCEPT("* * * * * */");
   CRON_EXPECT_EXCEPT("* * * * * /*");
   CRON_EXPECT_EXCEPT("* * * * * -/");
   CRON_EXPECT_EXCEPT("* * * * * /-");
   CRON_EXPECT_EXCEPT("* * * * * *-/");
   CRON_EXPECT_EXCEPT("* * * * * -*/");
   CRON_EXPECT_EXCEPT("* * * * * /-*");
   CRON_EXPECT_EXCEPT("* * * * * /*-");
   CRON_EXPECT_EXCEPT("* * * * * *2/");
   CRON_EXPECT_EXCEPT("* * * * * /2*");
   CRON_EXPECT_EXCEPT("* * * * * -2/");
   CRON_EXPECT_EXCEPT("* * * * * /2-");
   CRON_EXPECT_EXCEPT("* * * * * *2-/");
   CRON_EXPECT_EXCEPT("* * * * * -2*/");
   CRON_EXPECT_EXCEPT("* * * * * /2-*");
   CRON_EXPECT_EXCEPT("* * * * * /2*-");
}