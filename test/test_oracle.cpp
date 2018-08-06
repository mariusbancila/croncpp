#include "catch.hpp"
#include "croncpp.h"

#define ARE_EQUAL(x, y)          REQUIRE(x == y)
#define CRON_ORCL_EQUAL(x, y)    ARE_EQUAL(make_cron<cron::cron_oracle_traits>(x), make_cron<cron::cron_oracle_traits>(y))
#define CRON_EXPECT_EXCEPT(x)    REQUIRE_THROWS_AS(make_cron<cron::cron_oracle_traits>(x), bad_cronexpr)
#define CRON_EXPECT_MSG(x, msg)  REQUIRE_THROWS_WITH(make_cron<cron::cron_oracle_traits>(x), msg)

using namespace cron;

TEST_CASE("oracle: check seconds", "[oracle]")
{
   CRON_ORCL_EQUAL("*/5 * * * * *", "0,5,10,15,20,25,30,35,40,45,50,55 * * * * *");
   CRON_ORCL_EQUAL("1/6 * * * * *", "1,7,13,19,25,31,37,43,49,55 * * * * *");
   CRON_ORCL_EQUAL("0/30 * * * * *", "0,30 * * * * *");
   CRON_ORCL_EQUAL("0-5 * * * * *", "0,1,2,3,4,5 * * * * *");
   CRON_ORCL_EQUAL("55/1 * * * * *", "55,56,57,58,59 * * * * *");
   CRON_ORCL_EQUAL("57,59 * * * * *", "57/2 * * * * *");
   CRON_ORCL_EQUAL("1,3,5 * * * * *", "1-6/2 * * * * *");
   CRON_ORCL_EQUAL("0,5,10,15 * * * * *", "0-15/5 * * * * *");
}

TEST_CASE("oracle: check minutes", "[oracle]")
{
   CRON_ORCL_EQUAL("* */5 * * * *", "* 0,5,10,15,20,25,30,35,40,45,50,55 * * * *");
   CRON_ORCL_EQUAL("* 1/6 * * * *", "* 1,7,13,19,25,31,37,43,49,55 * * * *");
   CRON_ORCL_EQUAL("* 0/30 * * * *", "* 0,30 * * * *");
   CRON_ORCL_EQUAL("* 0-5 * * * *", "* 0,1,2,3,4,5 * * * *");
   CRON_ORCL_EQUAL("* 55/1 * * * *", "* 55,56,57,58,59 * * * *");
   CRON_ORCL_EQUAL("* 57,59 * * * *", "* 57/2 * * * *");
   CRON_ORCL_EQUAL("* 1,3,5 * * * *", "* 1-6/2 * * * *");
   CRON_ORCL_EQUAL("* 0,5,10,15 * * * *", "* 0-15/5 * * * *");
}

TEST_CASE("oracle: check hours", "[oracle]")
{
   CRON_ORCL_EQUAL("* * */5 * * *", "* * 0,5,10,15,20 * * *");
   CRON_ORCL_EQUAL("* * 1/6 * * *", "* * 1,7,13,19 * * *");
   CRON_ORCL_EQUAL("* * 0/12 * * *", "* * 0,12 * * *");
   CRON_ORCL_EQUAL("* * 0-5 * * *", "* * 0,1,2,3,4,5 * * *");
   CRON_ORCL_EQUAL("* * 15/1 * * *", "* * 15,16,17,18,19,20,21,22,23 * * *");
   CRON_ORCL_EQUAL("* * 17,19,21,23 * * *", "* * 17/2 * * *");
   CRON_ORCL_EQUAL("* * 1,3,5 * * *", "* * 1-6/2 * * *");
   CRON_ORCL_EQUAL("* * 0,5,10,15 * * *", "* * 0-15/5 * * *");
}

TEST_CASE("oracle: check days of month", "[oracle]")
{
   CRON_ORCL_EQUAL("* * * 1-31 * *", "* * * 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 * *");
   CRON_ORCL_EQUAL("* * * 1/5 * *", "* * * 1,6,11,16,21,26,31 * *");
   CRON_ORCL_EQUAL("* * * 1,11,21,31 * *", "* * * 1-31/10 * *");
   CRON_ORCL_EQUAL("* * * */5 * *", "* * * 1,6,11,16,21,26,31 * *");
}

TEST_CASE("oracle: check months", "[oracle]")
{
   CRON_ORCL_EQUAL("* * * * 0,5,10 *", "* * * * 0/5 *");
   CRON_ORCL_EQUAL("* * * * 0-11 *", "* * * * 0,1,2,3,4,5,6,7,8,9,10,11 *");
   CRON_ORCL_EQUAL("* * * * 0-11/3 *", "* * * * 0,3,6,9 *");
   CRON_ORCL_EQUAL("* * * * */2 *", "* * * * 0,2,4,6,8,10 *");
   CRON_ORCL_EQUAL("* * * * 1/2 *", "* * * * 1,3,5,7,9,11 *");
   CRON_ORCL_EQUAL("* * * * 0 *", "* * * * JAN *");
   CRON_ORCL_EQUAL("* * * * 1 *", "* * * * FEB *");
   CRON_ORCL_EQUAL("* * * * 2 *", "* * * * mar *");
   CRON_ORCL_EQUAL("* * * * 3 *", "* * * * apr *");
   CRON_ORCL_EQUAL("* * * * 4 *", "* * * * may *");
   CRON_ORCL_EQUAL("* * * * 5 *", "* * * * Jun *");
   CRON_ORCL_EQUAL("* * * * 6 *", "* * * * Jul *");
   CRON_ORCL_EQUAL("* * * * 7 *", "* * * * auG *");
   CRON_ORCL_EQUAL("* * * * 8 *", "* * * * sEp *");
   CRON_ORCL_EQUAL("* * * * 9 *", "* * * * oCT *");
   CRON_ORCL_EQUAL("* * * * 10 *", "* * * * nOV *");
   CRON_ORCL_EQUAL("* * * * 11 *", "* * * * DEC *");
   CRON_ORCL_EQUAL("* * * * 0,FEB *", "* * * * JAN,1 *");
   CRON_ORCL_EQUAL("* * * * 0,5,10 *", "* * * * NOV,JUN,jan *");
   CRON_ORCL_EQUAL("* * * * 0,5,10 *", "* * * * jan,jun,nov *");
   CRON_ORCL_EQUAL("* * * * 0,5,10 *", "* * * * jun,jan,nov *");
   CRON_ORCL_EQUAL("* * * * JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC *", "* * * * 0,1,2,3,4,5,6,7,8,9,10,11 *");
   CRON_ORCL_EQUAL("* * * * JUL,AUG,SEP,OCT,NOV,DEC,JAN,FEB,MAR,APR,MAY,JUN *", "* * * * 0,1,2,3,4,5,6,7,8,9,10,11 *");
}

TEST_CASE("oracle: check days of week", "[oracle]")
{
   CRON_ORCL_EQUAL("* * * * * 1-7", "* * * * * 1,2,3,4,5,6,7");
   CRON_ORCL_EQUAL("* * * * * 1-7/2", "* * * * * 1,3,5,7");
   CRON_ORCL_EQUAL("* * * * * 1-7/3", "* * * * * 1,4,7");
   CRON_ORCL_EQUAL("* * * * * */3", "* * * * * 1,4,7");
   CRON_ORCL_EQUAL("* * * * * 1/3", "* * * * * 1,4,7");
   CRON_ORCL_EQUAL("* * * * * 1", "* * * * * SUN");
   CRON_ORCL_EQUAL("* * * * * 2", "* * * * * MON");
   CRON_ORCL_EQUAL("* * * * * 3", "* * * * * TUE");
   CRON_ORCL_EQUAL("* * * * * 4", "* * * * * WED");
   CRON_ORCL_EQUAL("* * * * * 5", "* * * * * THU");
   CRON_ORCL_EQUAL("* * * * * 6", "* * * * * FRI");
   CRON_ORCL_EQUAL("* * * * * 7", "* * * * * SAT");
   CRON_ORCL_EQUAL("* * * * * 1-7", "* * * * * SUN,MON,TUE,WED,THU,FRI,SAT");
   CRON_ORCL_EQUAL("* * * * * 1-7/2", "* * * * * SUN,TUE,THU,SAT");
   CRON_ORCL_EQUAL("* * * * * 1-7/3", "* * * * * SUN,WED,SAT");
   CRON_ORCL_EQUAL("* * * * * */3", "* * * * * SUN,WED,SAT");
   CRON_ORCL_EQUAL("* * * * * 2/3", "* * * * * MON,THU");
}

TEST_CASE("oracle: invalid months", "[oracle]")
{
   CRON_EXPECT_EXCEPT("* * * * 12 *");
   CRON_EXPECT_EXCEPT("* * * * -1 *");
   CRON_EXPECT_EXCEPT("* * * * -1-12 *");
}

TEST_CASE("oracle: invalid days of week", "[oracle]")
{
   CRON_EXPECT_EXCEPT("* * * * * 8");
   CRON_EXPECT_EXCEPT("* * * * * 0");
   CRON_EXPECT_EXCEPT("* * * * * 0-8");
}