# croncpp

**croncpp** is a C++ header-only cross-platform library for handling CRON expressions. Works with any compiler that supports C++11 or anything newer. It implements two basic operations: parsing an expression and computing the next occurence of the scheduled time.

[![CI](https://github.com/mariusbancila/croncpp/actions/workflows/ci.yml/badge.svg)](https://github.com/mariusbancila/croncpp/actions/workflows/ci.yml)

## CRON expressions
A CRON expression is a string composed of six fields (in some implementation seven) separated by a whites space representing a time schedule. The general form is the following (with the `years` being optional):

```
<seconds> <minutes> <hours> <days of month> <months> <days of week> <years>
```

The following values are allowed for these fields:

| Field | Required | Allowed value * | Allowed value (alternative 1) ** | Allowed value (alternative 2) *** | Allowed special characters |
| --- | --- | --- | --- | --- | --- |
| seconds | yes | 0-59 | 0-59 | 0-59 | `*` `,` `-` |
| minutes | yes | 0-59 | 0-59 | 0-59 | `*` `,` `-` |
| hours | yes | 0-23 | 0-23 | 0-23 | `*` `,` `-` |
| days of month | 1-31 | 1-31 | 1-31 | 1-31 | `*` `,` `-` `?` `L` `W` |
| months | yes | 1-12 | 0-11 | 1-12 | `*` `,` `-` |
| days of week | yes | 0-6 | 1-7 | 1-7 | `*` `,` `-` `?` `L` `#` |
| years | no | 1970-2099 | 1970-2099 | 1970-2099 | `*` `,` `-` |

\* - As described on Wikipedia [Cron](https://en.wikipedia.org/wiki/Cron)

** - As described on Oracle [Role Manager Integration Guide - A Cron Expressions](https://docs.oracle.com/cd/E12058_01/doc/doc.1014/e12030/cron_expressions.htm)

*** - As described for the Quartz scheduler [CronTrigger Tutorial](http://www.quartz-scheduler.org/documentation/quartz-1.x/tutorials/crontrigger)

The special characters have the following meaning:

| Special character | Meaning | Description |
| --- | --- | --- |
| `*` | all values | selects all values within a field |
| `?` | no specific value | specify one field and leave the other unspecified |
| `-` | range | specify ranges |
| `,` | comma | specify additional values |
| `/` | slash | speficy increments |
| `L` | last | last day of the month or last day of the week |
| `W` | weekday | the weekday nearest to the given day |
| `#` | nth |  specify the Nth day of the month |

**Note:** the `L`, `W` and `#` special characters are described here for completeness but are **not implemented** by croncpp. An expression that uses one of them is rejected by `make_cron()` with a `bad_cronexpr` exception.

**Note:** an expression describing a date that never occurs, such as `0 0 5 31 2 ?` for the 31st of February, is also rejected by `make_cron()` with a `bad_cronexpr` exception, because it has no next occurrence to compute. February is measured as a leap year, so the 29th is accepted and the 30th is not.

Examples: 

| CRON | Description |
| --- | --- |
| * * * * * * | Every second |
| */5 * * * * ? | Every 5 seconds |
| 0 */5 */2 * * ? | Every 5 minutes, every 2 hours |
| 0 */2 */2 ? */2 */2 | Every 2 minutes, every 2 hours, every 2 days of the week, every 2 months |
| 0 15 10 * * ? * | 10:15 AM every day |
| 0 0/5 14 * * ? | Every 5 minutes starting at 2 PM and ending at 2:55 PM, every day |
| 0 10,44 14 ? 3 WED | 2:10 PM and at 2:44 PM every Wednesday of March |
| 0 15 10 ? * MON-FRI | 10:15 AM every Monday, Tuesday, Wednesday, Thursday and Friday |
| 0 0 12 1/5 * ? | 12 PM every 5 days every month, starting on the first day of the month |
| 0 11 11 11 11 ? | Every November 11th at 11:11 AM |

## croncpp library

To parse a CRON expression use `make_cron()` as follows:

```
try
{
   auto cron = cron::make_cron("* 0/5 * * * ?");
}
catch (cron::bad_cronexpr const & ex)
{
   std::cerr << ex.what() << '\n';
}
```

`make_cron()` returns an object of the type `cronexpr`. The actual content of this object is not of real interest and, in fact, all its details are private. You can consider this as an implementation detail object that contains the necessary information for a CRON expression, in order to compute the next occurence of the time schedule, which is the actual important operation we are interested in.

To get the next occurence of the time schedule use the `cron_next()` function as follows:

```
try
{
   auto cron = cron::make_cron("* 0/5 * * * ?");
   
   std::time_t now = std::time(0);
   std::time_t next = cron::cron_next(cron, now);   
}
catch (cron::bad_cronexpr const & ex)
{
   std::cerr << ex.what() << '\n';
}
```

Alternatively, you can use `std::tm` instead of `std::time_t`:

```
try
{
   auto cron = cron::make_cron("* 0/5 * * * ?");
   
   std::tm time = cron::utils::to_tm("2018-08-08 20:30:45");
   std::tm next = cron::cron_next(cron, time);
}
catch (cron::bad_cronexpr const & ex)
{
   std::cerr << ex.what() << '\n';
}
```

A `std::chrono::system_clock::time_point` overload is also available, but because `cron_next()` converts the time point to `time_t` using `to_time_t()`, fractional seconds are truncated.
If you are working from a known trigger instant and a sub-second drift could move the time point below the exact second, use `cron_next_ceil()` to round up to the next whole second before computing the next occurrence.

```
try
{
   auto cron = cron::make_cron("* 0/5 * * * ?");

   auto now = std::chrono::system_clock::now();
   auto next = cron::cron_next(cron, now);
   auto next_ceil = cron::cron_next_ceil(cron, now);
}
catch (cron::bad_cronexpr const & ex)
{
   std::cerr << ex.what() << '\n';
}
```

When you use these functions as shown above you implicitly use the standard supported values for the fields, as described in the first section. However, you can use any other settings. The ones provided with the library are called `cron_standard_traits`, `cron_oracle_traits` and `cron_quartz_traits` (coresponding to the aforementioned settings).

```
try
{
   auto cron = cron::make_cron<cron_quartz_traits>("* 0/5 * * * ?");
   
   std::time_t now = std::time(0);
   std::time_t next = cron::cron_next<cron_quartz_traits>(cron, now);   
}
catch (cron::bad_cronexpr const & ex)
{
   std::cerr << ex.what() << '\n';
}
```

There are two functions that convert the `cronexpr` object to a string:
* `to_cronstr()` returns the original cron expression text from with the object was created.
* `to_string()` returns a string format of the representation of the cron expression.
```
auto cex = make_cron("* * * * * *");

assert(to_cronstr(cex) == "* * * * * *");
assert(to_string(cex) == "111111111111111111111111111111111111111111111111111111111111 111111111111111111111111111111111111111111111111111111111111 111111111111111111111111 1111111111111111111111111111111 111111111111 1111111");
```

## Time zones and daylight saving time

croncpp evaluates expressions in **local time**. It has no time zone database of its own: `std::tm` values are converted with `std::mktime` and `localtime`, so the zone in effect is whatever the C runtime reports, which on most systems is controlled by the `TZ` environment variable. There is no UTC mode; to schedule in UTC, run the process with `TZ` set to UTC.

This matters when the local clock is not continuous, which happens twice a year in zones that observe daylight saving time.

**When the clock jumps forward**, some local times do not occur at all. An expression naming a time inside the gap does not fire that day; the next occurrence is on the following day.

```
// in a zone where the clock jumps from 02:00 to 03:00 on 2025-03-09
auto cex = cron::make_cron("0 30 2 * * *");         // every day at 02:30
auto tm  = cron::utils::to_tm("2025-03-09 01:00:00");

// 02:30 does not exist on the 9th, so the next occurrence is on the 10th
assert(cron::utils::to_string(cron::cron_next(cex, tm)) == "2025-03-10 02:30:00");
```

**When the clock goes back**, some local times occur twice, and the two are different instants an hour apart. Which of them `cron_next` returns depends on how the platform's `mktime` resolves an ambiguous local time, and that differs between implementations. croncpp does not attempt to hide the difference. What it does guarantee is that

* the result is a time matching the expression that is **strictly later than the one asked about**, so a caller that repeatedly feeds the previous result back in always makes progress and never repeats a value, and
* the `std::tm`, `std::time_t` and `std::chrono::system_clock::time_point` overloads all name the **same instant** for the same question, including inside the repeated hour.

## Benchmarks

The following results are the average (in microseconds) for running the benchmark program ten times on Windows and Mac with different compilers (all with release settings).

| VC++ 32-bit| VC++ 64-bit | GCC 32-bit | GCC 64-bit | Clang 64-bit |
| --- | --- | --- | --- | --- |
| 11.52 | 8.30 | 8.95 | 7.03 | 4.48 |

VC++ 15.7.4 running on 
* Windows 10 Enterprise build 17134
* Intel Core i7, 2.67 GHz, 1 CPU / 4 cores / 8 logical, 6 RAM
  
GCC 8.1.0 / Clang LLVM 9.1.0 running on
* macOS 10.13.5
* Intel Core i7, 1.7 GHz, 1 CPU / 2 cores, 8 GB RAM

![CRON parsin](res/cron_parsing.png)

## Credits

This library implementation is based on [ccronexpr](https://github.com/staticlibs/ccronexpr) ANSI C library, which in turn is based on the implementation of [CronSequenceGenerator](https://github.com/spring-projects/spring-framework/blob/babbf6e8710ab937cd05ece20270f51490299270/spring-context/src/main/java/org/springframework/scheduling/support/CronSequenceGenerator.java) from Spring Framework.
