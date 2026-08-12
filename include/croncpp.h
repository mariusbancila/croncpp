#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <bitset>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <limits>
#include <type_traits>

#if __cplusplus > 201402L
#include <string_view>
#define CRONCPP_IS_CPP17
#endif

namespace cron
{
#ifdef CRONCPP_IS_CPP17
   #define  CRONCPP_STRING_VIEW       std::string_view
   #define  CRONCPP_STRING_VIEW_NPOS  std::string_view::npos
   #define  CRONCPP_CONSTEXPTR        constexpr
#else
   #define  CRONCPP_STRING_VIEW       std::string const &
   #define  CRONCPP_STRING_VIEW_NPOS  std::string::npos
   #define  CRONCPP_CONSTEXPTR
#endif

   using cron_int  = uint8_t;

   constexpr std::time_t INVALID_TIME = static_cast<std::time_t>(-1);

   constexpr size_t INVALID_INDEX = static_cast<size_t>(-1);

   // The years a cronexpr can hold. cronexpr is not a template, so this range
   // is fixed for every traits type; a traits type may accept a narrower range
   // but not a wider one.
   constexpr int    CRON_YEAR_BASE  = 1970;
   constexpr size_t CRON_YEAR_COUNT = 130; // through 2099, as in Quartz

   // A traits type opts into the optional year field by declaring
   // CRON_MIN_YEARS and CRON_MAX_YEARS. One that does not, as any written
   // before the field existed, keeps taking six fields and rejects a seventh.
   template <typename...> struct make_void { using type = void; };
   template <typename... Ts> using void_t = typename make_void<Ts...>::type;

   template <typename Traits, typename = void>
   struct supports_years : std::false_type {};

   template <typename Traits>
   struct supports_years<Traits, void_t<decltype(Traits::CRON_MIN_YEARS)>>
      : std::true_type {};

   // What an expression means when it restricts both the day of month and the
   // day of week, which the two dialects croncpp follows answer differently.
   enum class day_field_rule
   {
      intersect, // a date has to match both fields
      either,    // a date matching either field is a match, as in POSIX cron
      reject     // the expression is an error, as in Quartz
   };

   // A traits type states its rule by declaring CRON_DAY_FIELD_RULE. One that
   // does not, as any written before the rule existed, keeps intersecting.
   template <typename Traits, typename = void>
   struct day_rule
      : std::integral_constant<day_field_rule, day_field_rule::intersect> {};

   template <typename Traits>
   struct day_rule<Traits, void_t<decltype(Traits::CRON_DAY_FIELD_RULE)>>
      : std::integral_constant<day_field_rule, Traits::CRON_DAY_FIELD_RULE> {};

   class cronexpr;

   namespace detail
   {
      enum class cron_field
      {
         second,
         minute,
         hour_of_day,
         day_of_week,
         day_of_month,
         month,
         year
      };

      template <typename Traits>
      static bool find_next(cronexpr const & cex,
                            std::tm& date,
                            size_t const dot);

      // The day fields accept qualifiers that a bitset cannot express, because
      // the day they select depends on the month: "the last one", "the third
      // Friday", "the weekday nearest the 15th". They are held alongside the
      // bitsets and applied when a candidate date is tested.
      struct day_of_month_options
      {
         bool     last = false;            // L, the last day of the month
         bool     nearest_weekday = false; // W, the nearest Monday to Friday
         cron_int day = 0;                 // the day W applies to, 0 for LW
         bool     restricted = false;      // the field was neither * nor ?
      };

      struct day_of_week_options
      {
         cron_int nth = 0;         // #, the 1st to 5th such weekday of the month
         bool     last = false;    // L, the last such weekday of the month
         bool     restricted = false; // the field was neither * nor ?
      };
   }

   struct bad_cronexpr : public std::runtime_error
   {
   public:
      explicit bad_cronexpr(CRONCPP_STRING_VIEW message) :
         std::runtime_error(message.data())
      {}
   };


   struct cron_standard_traits
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

      // POSIX cron: with both day fields restricted, either may match
      static const day_field_rule CRON_DAY_FIELD_RULE = day_field_rule::either;

      static const int CRON_MIN_YEARS = 1970;
      static const int CRON_MAX_YEARS = 2099;

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

   struct cron_oracle_traits
   {
      static const cron_int CRON_MIN_SECONDS = 0;
      static const cron_int CRON_MAX_SECONDS = 59;

      static const cron_int CRON_MIN_MINUTES = 0;
      static const cron_int CRON_MAX_MINUTES = 59;

      static const cron_int CRON_MIN_HOURS = 0;
      static const cron_int CRON_MAX_HOURS = 23;

      static const cron_int CRON_MIN_DAYS_OF_WEEK = 1;
      static const cron_int CRON_MAX_DAYS_OF_WEEK = 7;

      static const cron_int CRON_MIN_DAYS_OF_MONTH = 1;
      static const cron_int CRON_MAX_DAYS_OF_MONTH = 31;

      static const cron_int CRON_MIN_MONTHS = 0;
      static const cron_int CRON_MAX_MONTHS = 11;

      static const cron_int CRON_MAX_YEARS_DIFF = 4;

      // as in Quartz, one of the two day fields has to be ?
      static const day_field_rule CRON_DAY_FIELD_RULE = day_field_rule::reject;

      static const int CRON_MIN_YEARS = 1970;
      static const int CRON_MAX_YEARS = 2099;

#ifdef CRONCPP_IS_CPP17
      static const inline std::vector<std::string> DAYS = { "NIL", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
      static const inline std::vector<std::string> MONTHS = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
#else

      static std::vector<std::string>& DAYS()
      {
         static std::vector<std::string> days = { "NIL", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
         return days;
      }

      static std::vector<std::string>& MONTHS()
      {
         static std::vector<std::string> months = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
         return months;
      }
#endif
   };

   struct cron_quartz_traits
   {
      static const cron_int CRON_MIN_SECONDS = 0;
      static const cron_int CRON_MAX_SECONDS = 59;

      static const cron_int CRON_MIN_MINUTES = 0;
      static const cron_int CRON_MAX_MINUTES = 59;

      static const cron_int CRON_MIN_HOURS = 0;
      static const cron_int CRON_MAX_HOURS = 23;

      static const cron_int CRON_MIN_DAYS_OF_WEEK = 1;
      static const cron_int CRON_MAX_DAYS_OF_WEEK = 7;

      static const cron_int CRON_MIN_DAYS_OF_MONTH = 1;
      static const cron_int CRON_MAX_DAYS_OF_MONTH = 31;

      static const cron_int CRON_MIN_MONTHS = 1;
      static const cron_int CRON_MAX_MONTHS = 12;

      static const cron_int CRON_MAX_YEARS_DIFF = 4;

      // Quartz requires one of the two day fields to be ?
      static const day_field_rule CRON_DAY_FIELD_RULE = day_field_rule::reject;

      static const int CRON_MIN_YEARS = 1970;
      static const int CRON_MAX_YEARS = 2099;

#ifdef CRONCPP_IS_CPP17
      static const inline std::vector<std::string> DAYS = { "NIL", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
      static const inline std::vector<std::string> MONTHS = { "NIL", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
#else
      static std::vector<std::string>& DAYS()
      {
         static std::vector<std::string> days = { "NIL", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
         return days;
      }

      static std::vector<std::string>& MONTHS()
      {
         static std::vector<std::string> months = { "NIL", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
         return months;
      }
#endif
   };

   class cronexpr;

   template <typename Traits = cron_standard_traits>
   static cronexpr make_cron(CRONCPP_STRING_VIEW expr);

   class cronexpr
   {
      std::bitset<60> seconds;
      std::bitset<60> minutes;
      std::bitset<24> hours;
      std::bitset<7>  days_of_week;
      std::bitset<31> days_of_month;
      std::bitset<12> months;
      std::bitset<CRON_YEAR_COUNT> years;
      std::string     expr;

      detail::day_of_month_options dom_options;
      detail::day_of_week_options  dow_options;

      friend bool operator==(cronexpr const & e1, cronexpr const & e2);
      friend bool operator!=(cronexpr const & e1, cronexpr const & e2);

      template <typename Traits>
      friend bool detail::find_next(cronexpr const & cex,
                                    std::tm& date,
                                    size_t const dot);

      friend std::string to_cronstr(cronexpr const& cex);
      friend std::string to_string(cronexpr const & cex);

      template <typename Traits>
      friend cronexpr make_cron(CRONCPP_STRING_VIEW expr);

   public:
      bool empty() const noexcept
      {
         // a day of month selected by L or W leaves the bitset empty, because
         // which day it is depends on the month
         bool const no_day_of_month =
            days_of_month.none() &&
            !dom_options.last &&
            !dom_options.nearest_weekday;

         return
            seconds.none() ||
            minutes.none() ||
            hours.none() ||
            days_of_week.none() ||
            no_day_of_month ||
            months.none() ||
            years.none();
      }
   };

   inline bool operator==(cronexpr const & e1, cronexpr const & e2)
   {
      return
         e1.seconds == e2.seconds &&
         e1.minutes == e2.minutes &&
         e1.hours == e2.hours &&
         e1.days_of_week == e2.days_of_week &&
         e1.days_of_month == e2.days_of_month &&
         e1.months == e2.months &&
         e1.years == e2.years &&
         e1.dom_options.last == e2.dom_options.last &&
         e1.dom_options.nearest_weekday == e2.dom_options.nearest_weekday &&
         e1.dom_options.day == e2.dom_options.day &&
         e1.dom_options.restricted == e2.dom_options.restricted &&
         e1.dow_options.nth == e2.dow_options.nth &&
         e1.dow_options.last == e2.dow_options.last &&
         e1.dow_options.restricted == e2.dow_options.restricted;
   }

   inline bool operator!=(cronexpr const & e1, cronexpr const & e2)
   {
      return !(e1 == e2);
   }

   inline std::string to_string(cronexpr const & cex)
   {
      return
         cex.seconds.to_string() + " " +
         cex.minutes.to_string() + " " +
         cex.hours.to_string() + " " +
         cex.days_of_month.to_string() + " " +
         cex.months.to_string() + " " +
         cex.days_of_week.to_string();
   }

   inline std::string to_cronstr(cronexpr const& cex)
   {
      return cex.expr;
   }

   namespace utils
   {
      inline std::time_t tm_to_time(std::tm& date)
      {
         return std::mktime(&date);
      }

      inline std::tm* time_to_tm(std::time_t const * date, std::tm* const out)
      {
#ifdef _WIN32
         errno_t err = localtime_s(out, date);
         return 0 == err ? out : nullptr;
#else
         return localtime_r(date, out);
#endif
      }

      inline std::tm to_tm(CRONCPP_STRING_VIEW time)
      {
         std::tm result;
#if __cplusplus > 201103L
         std::istringstream str(time.data());
         str.imbue(std::locale(setlocale(LC_ALL, nullptr)));

         str >> std::get_time(&result, "%Y-%m-%d %H:%M:%S");
         if (str.fail()) throw std::runtime_error("Parsing date failed!");
#else
         int year = 1900;
         int month = 1;
         int day = 1;
         int hour = 0;
         int minute = 0;
         int second = 0;
         sscanf(time.data(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
         result.tm_year = year - 1900;
         result.tm_mon = month - 1;
         result.tm_mday = day;
         result.tm_hour = hour;
         result.tm_min = minute;
         result.tm_sec = second;
#endif
         result.tm_isdst = -1; // DST info not available

         return result;
      }

      inline std::string to_string(std::tm const & tm)
      {
#if __cplusplus > 201103L
         std::ostringstream str;
         str.imbue(std::locale(setlocale(LC_ALL, nullptr)));
         str << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
         if (str.fail()) throw std::runtime_error("Writing date failed!");

         return str.str();
#else
         char buff[70] = {0};
         strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", &tm);
         return std::string(buff);
#endif
      }

      inline std::string to_upper(std::string text)
      {
         std::transform(std::begin(text), std::end(text),
            std::begin(text), [](char const c) { return static_cast<char>(std::toupper(c)); });

         return text;
      }

      static std::vector<std::string> split(CRONCPP_STRING_VIEW text, char const delimiter)
      {
         std::vector<std::string> tokens;
         std::string token;
         std::istringstream tokenStream(text.data());
         while (std::getline(tokenStream, token, delimiter))
         {
            tokens.push_back(token);
         }
         return tokens;
      }

      CRONCPP_CONSTEXPTR inline bool contains(CRONCPP_STRING_VIEW text, char const ch) noexcept
      {
         return CRONCPP_STRING_VIEW_NPOS != text.find_first_of(ch);
      }
   }

   namespace detail
   {

      inline bool is_leap_year(int const year)
      {
         return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
      }

      // month is 0 based, as in std::tm::tm_mon
      inline int days_in_month(int const year, int const month)
      {
         static int const days[12] =
            { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

         if (month == 1 && is_leap_year(year)) return 29;

         return days[month];
      }

      template <typename T>
      inline T to_cron_number(CRONCPP_STRING_VIEW text)
      {
         if (text.empty())
            throw bad_cronexpr("Cron field value cannot be empty");

         for (auto const ch : text)
         {
            if (ch < '0' || ch > '9')
            {
               if (ch == 'L' || ch == 'W' || ch == '#')
                  throw bad_cronexpr(
                     std::string("Special character '") + ch + "' is not valid in this field");

               throw bad_cronexpr(
                  "Invalid character in cron field: " + std::string(text));
            }
         }

         try
         {
            auto const value = std::stoul(std::string(text));
            if (value > static_cast<unsigned long>((std::numeric_limits<T>::max)()))
               throw bad_cronexpr("Cron field value is out of range");

            return static_cast<T>(value);
         }
         catch (std::invalid_argument const & ex)
         {
            throw bad_cronexpr(ex.what());
         }
         catch (std::out_of_range const & ex)
         {
            throw bad_cronexpr(ex.what());
         }
      }

      inline bool is_field_separator(char const ch)
      {
         return ch == ',' || ch == '-' || ch == '/';
      }

      inline cron_int to_cron_int(CRONCPP_STRING_VIEW text)
      {
         return to_cron_number<cron_int>(text);
      }

      static std::string replace_ordinals(
         std::string text,
         std::vector<std::string> const & replacement)
      {
         for (size_t i = 0; i < replacement.size(); ++i)
         {
            std::string const & name = replacement[i];
            if (name.empty()) continue;

            std::string const value = std::to_string(i);

            size_t pos = text.find(name);
            while (std::string::npos != pos)
            {
               // Only a name standing on its own is a name. Replacing it
               // wherever it appears turns "JAN1" into "11", which is a
               // perfectly good month number and quietly means November.
               size_t const end = pos + name.size();

               bool const at_start = pos == 0 || is_field_separator(text[pos - 1]);
               bool const at_end = end == text.size() || is_field_separator(text[end]);

               if (at_start && at_end)
               {
                  text.replace(pos, name.size(), value);
                  pos = text.find(name, pos + value.size());
               }
               else
               {
                  pos = text.find(name, pos + 1);
               }
            }
         }

         return text;
      }

      template <typename T>
      static std::pair<T, T> make_range(
         CRONCPP_STRING_VIEW field,
         T const minval,
         T const maxval)
      {
         T first = 0;
         T last = 0;
         if (field.size() == 1 && field[0] == '*')
         {
            first = minval;
            last = maxval;
         }
         else if (!utils::contains(field, '-'))
         {
            first = to_cron_number<T>(field);
            last = first;
         }
         else
         {
            auto parts = utils::split(field, '-');
            if (parts.size() != 2)
               throw bad_cronexpr("Specified range requires two fields");

            first = to_cron_number<T>(parts[0]);
            last = to_cron_number<T>(parts[1]);
         }

         if (first > maxval || last > maxval)
         {
            throw bad_cronexpr("Specified range exceeds maximum");
         }
         if (first < minval || last < minval)
         {
            throw bad_cronexpr("Specified range is less than minimum");
         }
         if (first > last)
         {
            throw bad_cronexpr("Specified range start exceeds range end");
         }

         return { first, last };
      }

      template <typename T, size_t N>
      static void set_cron_field(
         CRONCPP_STRING_VIEW value,
         std::bitset<N>& target,
         T const minval,
         T const maxval)
      {
         if(value.length() > 0 && value[value.length()-1] == ',')
            throw bad_cronexpr("Value cannot end with comma");

         auto fields = utils::split(value, ',');
         if (fields.empty())
            throw bad_cronexpr("Expression parsing error");

         for (auto const & field : fields)
         {
            if (!utils::contains(field, '/'))
            {
#ifdef CRONCPP_IS_CPP17
               auto[first, last] = detail::make_range<T>(field, minval, maxval);
#else
               auto range = detail::make_range<T>(field, minval, maxval);
               auto first = range.first;
               auto last = range.second;
#endif
               for (T i = first - minval; i <= last - minval; ++i)
               {
                  target.set(i);
               }
            }
            else
            {
               auto parts = utils::split(field, '/');
               if (parts.size() != 2)
                  throw bad_cronexpr("Incrementer must have two fields");

#ifdef CRONCPP_IS_CPP17
               auto[first, last] = detail::make_range<T>(parts[0], minval, maxval);
#else
               auto range = detail::make_range<T>(parts[0], minval, maxval);
               auto first = range.first;
               auto last = range.second;
#endif

               if (!utils::contains(parts[0], '-'))
               {
                  last = maxval;
               }

               auto delta = detail::to_cron_number<T>(parts[1]);
               if(delta <= 0)
                  throw bad_cronexpr("Incrementer must be a positive value");

               for (T i = first - minval; i <= last - minval; i += delta)
               {
                  target.set(i);
               }
            }
         }
      }

      inline bool has_list_or_range(std::string const & value)
      {
         return
            utils::contains(value, ',') ||
            utils::contains(value, '-') ||
            utils::contains(value, '/');
      }

      template <typename Traits>
      static void set_cron_days_of_week(
         std::string value,
         std::bitset<7>& target,
         day_of_week_options & options)
      {
         auto days = utils::to_upper(value);

         if (days.size() == 1 && days[0] == '?')
            days[0] = '*';

         // Whether the field names any particular day at all. Only * and ?
         // leave it open; a list, a range, or even 1-31 restricts it, which is
         // the distinction POSIX cron draws between the two day fields.
         options.restricted = !(days.size() == 1 && days[0] == '*');

         // On its own, L means Saturday, as in Quartz. After a weekday it
         // means the last such weekday of the month, and # selects which one.
         if (days == "L")
         {
            days = std::to_string(Traits::CRON_MAX_DAYS_OF_WEEK);
         }
         else if (utils::contains(days, '#') || (!days.empty() && days.back() == 'L'))
         {
            if (has_list_or_range(days))
               throw bad_cronexpr(
                  "The L and # special characters apply to a single day of week");

            if (days.back() == 'L')
            {
               options.last = true;
               days.pop_back();
            }
            else
            {
               auto const parts = utils::split(days, '#');
               if (parts.size() != 2)
                  throw bad_cronexpr("The # special character must have two fields");

               auto const nth = to_cron_int(parts[1]);
               if (nth < 1 || nth > 5)
                  throw bad_cronexpr(
                     "The # special character must select the 1st to the 5th weekday");

               options.nth = nth;
               days = parts[0];
            }
         }

         auto days_replaced = detail::replace_ordinals(
            days,
#ifdef CRONCPP_IS_CPP17
            Traits::DAYS
#else
            Traits::DAYS()
#endif
         );

         set_cron_field(
            days_replaced,
            target,
            Traits::CRON_MIN_DAYS_OF_WEEK,
            Traits::CRON_MAX_DAYS_OF_WEEK);

         if ((options.nth != 0 || options.last) && target.count() != 1)
            throw bad_cronexpr(
               "The L and # special characters apply to a single day of week");
      }

      template <typename Traits>
      static void set_cron_days_of_month(
         std::string value,
         std::bitset<31>& target,
         day_of_month_options & options)
      {
         auto days = utils::to_upper(value);

         if (days.size() == 1 && days[0] == '?')
            days[0] = '*';

         options.restricted = !(days.size() == 1 && days[0] == '*');

         if (utils::contains(days, 'L') || utils::contains(days, 'W'))
         {
            if (has_list_or_range(days))
               throw bad_cronexpr(
                  "The L and W special characters apply to a single day of month");

            if (days == "L")
            {
               options.last = true;
               return;
            }

            if (days == "LW")
            {
               options.last = true;
               options.nearest_weekday = true;
               return;
            }

            if (days.back() != 'W')
               throw bad_cronexpr("Invalid character in cron field: " + value);

            days.pop_back();

            auto const day = to_cron_int(days);
            if (day < Traits::CRON_MIN_DAYS_OF_MONTH ||
                day > Traits::CRON_MAX_DAYS_OF_MONTH)
               throw bad_cronexpr("Specified range exceeds maximum");

            options.nearest_weekday = true;
            options.day = day;
            return;
         }

         set_cron_field(
            days,
            target,
            Traits::CRON_MIN_DAYS_OF_MONTH,
            Traits::CRON_MAX_DAYS_OF_MONTH);
      }

      template <typename Traits>
      static void set_cron_month(
         std::string value,
         std::bitset<12>& target)
      {
         auto month = utils::to_upper(value);
         auto month_replaced = replace_ordinals(
            month,
#ifdef CRONCPP_IS_CPP17
            Traits::MONTHS
#else
            Traits::MONTHS()
#endif
         );

         set_cron_field(
            month_replaced,
            target,
            Traits::CRON_MIN_MONTHS,
            Traits::CRON_MAX_MONTHS);
      }

      // The optional year field. The primary template is used when the traits
      // type declares a year range; the specialisation keeps traits written
      // before the field existed compiling, with a seventh field rejected.
      template <typename Traits, bool = supports_years<Traits>::value>
      struct year_field
      {
         static void set(
            std::string const & value,
            std::bitset<CRON_YEAR_COUNT> & target)
         {
            static_assert(Traits::CRON_MIN_YEARS >= CRON_YEAR_BASE,
               "the traits accept years earlier than croncpp can store");
            static_assert(Traits::CRON_MAX_YEARS <
                          CRON_YEAR_BASE + static_cast<int>(CRON_YEAR_COUNT),
               "the traits accept years later than croncpp can store");

            // An unrestricted field means every year the traits allow, which
            // may be narrower than the range croncpp stores.
            if (value == "*")
            {
               set_all(target);
               return;
            }

            // indexed from CRON_YEAR_BASE, so that the stored range does not
            // depend on the traits
            set_cron_field<int>(
               value,
               target,
               CRON_YEAR_BASE,
               static_cast<int>(CRON_YEAR_BASE + CRON_YEAR_COUNT) - 1);

            // a traits type may accept a narrower range than croncpp stores
            for (size_t i = 0; i < target.size(); ++i)
            {
               if (!target.test(i)) continue;

               int const year = static_cast<int>(i) + CRON_YEAR_BASE;
               if (year < Traits::CRON_MIN_YEARS || year > Traits::CRON_MAX_YEARS)
                  throw bad_cronexpr("Specified year is out of range");
            }
         }

         // used when the expression leaves the field out altogether
         static void set_all(std::bitset<CRON_YEAR_COUNT> & target)
         {
            for (int year = Traits::CRON_MIN_YEARS;
                 year <= Traits::CRON_MAX_YEARS;
                 ++year)
               target.set(static_cast<size_t>(year - CRON_YEAR_BASE));
         }
      };

      template <typename Traits>
      struct year_field<Traits, false>
      {
         static void set(
            std::string const &,
            std::bitset<CRON_YEAR_COUNT> &)
         {
            throw bad_cronexpr("These traits do not support a year field");
         }

         // these traits have no notion of a year, so no year is excluded
         static void set_all(std::bitset<CRON_YEAR_COUNT> & target)
         {
            target.set();
         }
      };

      template <size_t N>
      inline size_t next_set_bit(
         std::bitset<N> const & target,
         size_t /*minimum*/,
         size_t /*maximum*/,
         size_t offset)
      {
         for (auto i = offset; i < N; ++i)
         {
            if (target.test(i)) return i;
         }

         return INVALID_INDEX;
      }

      // Reports whether any day the expression asks for can occur in any month
      // it asks for. February is measured as a leap year, so the 29th counts
      // as reachable and the 30th does not.
      //
      // Both bitsets are indexed from the traits minimum, and every traits
      // type numbers January and the first of the month as that minimum, so
      // bit 0 is January and day 1 whatever the traits are.
      inline bool has_reachable_date(
         std::bitset<31> const & days_of_month,
         std::bitset<12> const & months,
         day_of_month_options const & options)
      {
         // the last day of a month always exists, whichever month it is
         if (options.last) return true;

         static int const last_day_of[12] =
            { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

         for (size_t month = 0; month < months.size(); ++month)
         {
            if (!months.test(month)) continue;

            if (options.nearest_weekday)
            {
               if (static_cast<int>(options.day) <= last_day_of[month]) return true;
               continue;
            }

            for (size_t day = 0; day < days_of_month.size(); ++day)
            {
               if (days_of_month.test(day) &&
                   static_cast<int>(day) + 1 <= last_day_of[month])
                  return true;
            }
         }

         return false;
      }

      inline int field_value(
         std::tm const & date,
         cron_field const field)
      {
         switch (field)
         {
         case cron_field::second:       return date.tm_sec;
         case cron_field::minute:       return date.tm_min;
         case cron_field::hour_of_day:  return date.tm_hour;
         case cron_field::day_of_week:  return date.tm_wday;
         case cron_field::day_of_month: return date.tm_mday;
         case cron_field::month:        return date.tm_mon;
         case cron_field::year:         return date.tm_year;
         }

         return -1;
      }

      inline void add_to_field(
         std::tm& date,
         cron_field const field,
         int const val)
      {
         switch (field)
         {
         case cron_field::second:
            date.tm_sec += val;
            break;
         case cron_field::minute:
            date.tm_min += val;
            break;
         case cron_field::hour_of_day:
            date.tm_hour += val;
            break;
         case cron_field::day_of_week:
         case cron_field::day_of_month:
            date.tm_mday += val;
            break;
         case cron_field::month:
            date.tm_mon += val;
            break;
         case cron_field::year:
            date.tm_year += val;
            break;
         }

         // whatever the field, the time that comes out may fall on the other
         // side of a DST transition from the one that went in, so the flag is
         // no longer known and mktime has to work it out
         date.tm_isdst = -1;

         if (INVALID_TIME == utils::tm_to_time(date))
            throw bad_cronexpr("Invalid time expression");
      }

      inline void set_field(
         std::tm& date,
         cron_field const field,
         int const val)
      {
         switch (field)
         {
         case cron_field::second:
            date.tm_sec = val;
            break;
         case cron_field::minute:
            date.tm_min = val;
            break;
         case cron_field::hour_of_day:
            date.tm_hour = val;
            break;
         case cron_field::day_of_week:
            date.tm_wday = val;
            break;
         case cron_field::day_of_month:
            date.tm_mday = val;
            break;
         case cron_field::month:
            // Moving to a month shorter than the day currently set would roll
            // over into the month after it: the 31st of February becomes the
            // 3rd of March, and the search then misses the month it was aiming
            // for. The day is searched again from the start of the month
            // anyway, so begin it at the 1st.
            date.tm_mday = 1;
            date.tm_mon = val;
            break;
         case cron_field::year:
            date.tm_year = val;
            break;
         }

         date.tm_isdst = -1;

         if (INVALID_TIME == utils::tm_to_time(date))
            throw bad_cronexpr("Invalid time expression");
      }

      inline void reset_field(
         std::tm& date,
         cron_field const field)
      {
         switch (field)
         {
         case cron_field::second:
            date.tm_sec = 0;
            break;
         case cron_field::minute:
            date.tm_min = 0;
            break;
         case cron_field::hour_of_day:
            date.tm_hour = 0;
            break;
         case cron_field::day_of_week:
            date.tm_wday = 0;
            break;
         case cron_field::day_of_month:
            date.tm_mday = 1;
            break;
         case cron_field::month:
            date.tm_mon = 0;
            break;
         case cron_field::year:
            date.tm_year = 0;
            break;
         }

         date.tm_isdst = -1;

         if (INVALID_TIME == utils::tm_to_time(date))
            throw bad_cronexpr("Invalid time expression");
      }

      inline void reset_all_fields(
         std::tm& date,
         std::bitset<7> const & marked_fields)
      {
         for (size_t i = 0; i < marked_fields.size(); ++i)
         {
            if (marked_fields.test(i))
               reset_field(date, static_cast<cron_field>(i));
         }
      }

      inline void mark_field(
         std::bitset<7> & orders,
         cron_field const field)
      {
         if (!orders.test(static_cast<size_t>(field)))
            orders.set(static_cast<size_t>(field));
      }

      template <size_t N>
      static size_t find_next(
         std::bitset<N> const & target,
         std::tm& date,
         unsigned int const minimum,
         unsigned int const maximum,
         unsigned int const value,
         cron_field const field,
         cron_field const next_field,
         std::bitset<7> const & marked_fields)
      {
         auto next_value = next_set_bit(target, minimum, maximum, value);
         if (INVALID_INDEX == next_value)
         {
            add_to_field(date, next_field, 1);
            reset_field(date, field);
            next_value = next_set_bit(target, minimum, maximum, 0);
         }

         if (INVALID_INDEX == next_value || next_value != value)
         {
            set_field(date, field, static_cast<int>(next_value));
            reset_all_fields(date, marked_fields);

            // The value asked for may not exist on this day: when the clock
            // jumps forward there is no 02:30 at all, and mktime answers with
            // some other time. Asking again would never make progress, so move
            // on to the next larger field instead.
            if (INVALID_INDEX != next_value &&
                field_value(date, field) != static_cast<int>(next_value))
            {
               add_to_field(date, next_field, 1);
               reset_field(date, field);

               next_value = next_set_bit(target, minimum, maximum, 0);
               if (INVALID_INDEX != next_value)
               {
                  set_field(date, field, static_cast<int>(next_value));
                  reset_all_fields(date, marked_fields);
               }
            }
         }

         return next_value;
      }

      // The Quartz rule for W: move to the nearest Monday to Friday without
      // leaving the month, so the 1st on a Saturday moves to the 3rd and the
      // last day on a Sunday moves two days back. Returns 0 when the day does
      // not exist in this month at all.
      inline int nearest_weekday_to(
         std::tm const & date,
         int const target,
         int const last)
      {
         if (target < 1 || target > last) return 0;

         int const offset = target - date.tm_mday;
         int const weekday = ((date.tm_wday + offset) % 7 + 7) % 7;

         if (weekday == 6) return target > 1 ? target - 1 : target + 2;
         if (weekday == 0) return target < last ? target + 1 : target - 2;

         return target;
      }

      template <typename Traits>
      inline bool matches_day_of_month(
         std::tm const & date,
         std::bitset<31> const & days_of_month,
         day_of_month_options const & options)
      {
         int const last = days_in_month(date.tm_year + 1900, date.tm_mon);

         if (options.last)
         {
            return options.nearest_weekday
               ? date.tm_mday == nearest_weekday_to(date, last, last)
               : date.tm_mday == last;
         }

         if (options.nearest_weekday)
            return date.tm_mday == nearest_weekday_to(date, options.day, last);

         return days_of_month.test(date.tm_mday - Traits::CRON_MIN_DAYS_OF_MONTH);
      }

      inline bool matches_day_of_week(
         std::tm const & date,
         std::bitset<7> const & days_of_week,
         day_of_week_options const & options)
      {
         if (!days_of_week.test(date.tm_wday)) return false;

         if (options.nth != 0)
            return (date.tm_mday - 1) / 7 + 1 == options.nth;

         if (options.last)
            return date.tm_mday + 7 >
                   days_in_month(date.tm_year + 1900, date.tm_mon);

         return true;
      }

      // Combines the two day fields according to the rule of the dialect the
      // traits describe. An unrestricted field matches every day, so only the
      // case where both name particular days can differ between the rules.
      template <typename Traits>
      inline bool matches_day(
         std::tm const & date,
         std::bitset<31> const & days_of_month,
         day_of_month_options const & dom_options,
         std::bitset<7> const & days_of_week,
         day_of_week_options const & dow_options)
      {
         bool const day_of_month =
            matches_day_of_month<Traits>(date, days_of_month, dom_options);
         bool const day_of_week =
            matches_day_of_week(date, days_of_week, dow_options);

         if (day_rule<Traits>::value == day_field_rule::either &&
             dom_options.restricted && dow_options.restricted)
            return day_of_month || day_of_week;

         return day_of_month && day_of_week;
      }

      template <typename Traits>
      static size_t find_next_day(
         std::tm& date,
         std::bitset<31> const & days_of_month,
         day_of_month_options const & dom_options,
         size_t day_of_month,
         std::bitset<7> const & days_of_week,
         day_of_week_options const & dow_options,
         std::bitset<7> const & marked_fields)
      {
         unsigned int count = 0;
         unsigned int maximum = 366;
         // The matchers read the day straight from date. tm_wday is always
         // 0 (Sunday) to 6 (Saturday) whatever the traits are, and is already
         // the bit index, because the cron value of Sunday is
         // CRON_MIN_DAYS_OF_WEEK and the bits are set at
         // value - CRON_MIN_DAYS_OF_WEEK.
         while (
            !matches_day<Traits>(
               date, days_of_month, dom_options, days_of_week, dow_options)
            && count++ < maximum)
         {
            add_to_field(date, cron_field::day_of_month, 1);

            day_of_month = date.tm_mday;

            reset_all_fields(date, marked_fields);
         }

         return day_of_month;
      }

      template <typename Traits>
      static bool find_next(cronexpr const & cex,
                            std::tm& date,
                            size_t const dot)
      {
         bool res = true;

         std::bitset<7> marked_fields{ 0 };
         std::bitset<7> empty_list{ 0 };

         unsigned int second = date.tm_sec;
         auto updated_second = find_next(
            cex.seconds,
            date,
            Traits::CRON_MIN_SECONDS,
            Traits::CRON_MAX_SECONDS,
            second,
            cron_field::second,
            cron_field::minute,
            empty_list);

         if (second == updated_second)
         {
            mark_field(marked_fields, cron_field::second);
         }

         unsigned int minute = date.tm_min;
         auto update_minute = find_next(
            cex.minutes,
            date,
            Traits::CRON_MIN_MINUTES,
            Traits::CRON_MAX_MINUTES,
            minute,
            cron_field::minute,
            cron_field::hour_of_day,
            marked_fields);
         if (minute == update_minute)
         {
            mark_field(marked_fields, cron_field::minute);
         }
         else
         {
            res = find_next<Traits>(cex, date, dot);
            if (!res) return res;
         }

         unsigned int hour = date.tm_hour;
         auto updated_hour = find_next(
            cex.hours,
            date,
            Traits::CRON_MIN_HOURS,
            Traits::CRON_MAX_HOURS,
            hour,
            cron_field::hour_of_day,
            cron_field::day_of_week,
            marked_fields);
         if (hour == updated_hour)
         {
            mark_field(marked_fields, cron_field::hour_of_day);
         }
         else
         {
            res = find_next<Traits>(cex, date, dot);
            if (!res) return res;
         }

         unsigned int day_of_month = date.tm_mday;
         auto updated_day_of_month = find_next_day<Traits>(
            date,
            cex.days_of_month,
            cex.dom_options,
            day_of_month,
            cex.days_of_week,
            cex.dow_options,
            marked_fields);
         if (day_of_month == updated_day_of_month)
         {
            mark_field(marked_fields, cron_field::day_of_month);
         }
         else
         {
            res = find_next<Traits>(cex, date, dot);
            if (!res) return res;
         }

         unsigned int month = date.tm_mon;
         auto updated_month = find_next(
            cex.months,
            date,
            Traits::CRON_MIN_MONTHS,
            Traits::CRON_MAX_MONTHS,
            month,
            cron_field::month,
            cron_field::year,
            marked_fields);
         if (month != updated_month)
         {
            // An expression that names its years bounds the search by itself,
            // and may legitimately reach further ahead than this cap allows.
            if (cex.years.all() && date.tm_year - dot > Traits::CRON_MAX_YEARS_DIFF)
               return false;

            res = find_next<Traits>(cex, date, dot);
            if (!res) return res;
         }

         if (!cex.years.all())
         {
            int const year = date.tm_year + 1900;
            size_t const index = year > CRON_YEAR_BASE
               ? static_cast<size_t>(year - CRON_YEAR_BASE)
               : 0;

            if (index >= CRON_YEAR_COUNT) return false;

            if (!cex.years.test(index))
            {
               auto const next_year =
                  next_set_bit(cex.years, 0, CRON_YEAR_COUNT, index);
               if (INVALID_INDEX == next_year) return false;

               // Jump straight to the start of that year rather than walking
               // the months towards it, which for a distant year would recurse
               // once per month along the way.
               std::tm start = std::tm();
               start.tm_year = static_cast<int>(next_year) + CRON_YEAR_BASE - 1900;
               start.tm_mon = 0;
               start.tm_mday = 1;
               start.tm_isdst = -1;

               if (INVALID_TIME == utils::tm_to_time(start)) return false;

               date = start;

               return find_next<Traits>(cex, date, date.tm_year);
            }
         }

         return res;
      }

      // The largest shift a DST transition applies to the clock.
      // It bounds the search below, which can only fail to move forward while it is inside such a transition.
      constexpr std::time_t CRON_MAX_DST_SHIFT = 2 * 60 * 60;

      // Finds the first time matching the expression that is strictly later
      // than the one given. find_next works on a local std::tm, and across a
      // DST transition mktime can map that local time onto an instant at or
      // before the one the search started from: the local clock moves forward
      // while the instant it names does not. The result is therefore checked
      // and the search restarted a second later until it really is in the
      // future, which also keeps a caller looping on cron_next from spinning
      // on the same value.
      template <typename Traits>
      static std::time_t find_next_after(
         cronexpr const & cex,
         std::time_t const original,
         std::tm & result)
      {
         for (std::time_t start = original; start - original <= CRON_MAX_DST_SHIFT; ++start)
         {
            std::tm date;
            if (utils::time_to_tm(&start, &date) == nullptr)
               return INVALID_TIME;

            if (!find_next<Traits>(cex, date, date.tm_year))
               return INVALID_TIME;

            std::time_t const calculated = utils::tm_to_time(date);
            if (INVALID_TIME == calculated)
               return INVALID_TIME;

            if (calculated > original)
            {
               // Derive the calendar time back from the instant rather than
               // handing out the one mktime normalized. When a local time is
               // ambiguous, because the clock went back, mktime may leave
               // tm_isdst describing the other of the two readings, and the
               // two overloads of cron_next would then answer with different
               // instants for the same call.
               if (utils::time_to_tm(&calculated, &result) == nullptr)
                  return INVALID_TIME;

               return calculated;
            }
         }

         return INVALID_TIME;
      }
   }

   template <typename Traits>
   static cronexpr make_cron(CRONCPP_STRING_VIEW expr)
   {
      cronexpr cex;

      if (expr.empty())
         throw bad_cronexpr("Invalid empty cron expression");

      auto fields = utils::split(expr, ' ');
      fields.erase(
         std::remove_if(std::begin(fields), std::end(fields),
            [](CRONCPP_STRING_VIEW s) {return s.empty(); }),
         std::end(fields));
      // the year is optional, and only for traits that declare a range for it
      bool const years_allowed = supports_years<Traits>::value;

      if (fields.size() != 6 && !(years_allowed && fields.size() == 7))
         throw bad_cronexpr(
            years_allowed
               ? "cron expression must have six or seven fields"
               : "cron expression must have six fields");

      detail::set_cron_field(fields[0], cex.seconds, Traits::CRON_MIN_SECONDS, Traits::CRON_MAX_SECONDS);
      detail::set_cron_field(fields[1], cex.minutes, Traits::CRON_MIN_MINUTES, Traits::CRON_MAX_MINUTES);
      detail::set_cron_field(fields[2], cex.hours, Traits::CRON_MIN_HOURS, Traits::CRON_MAX_HOURS);

      detail::set_cron_days_of_week<Traits>(fields[5], cex.days_of_week, cex.dow_options);

      detail::set_cron_days_of_month<Traits>(fields[3], cex.days_of_month, cex.dom_options);

      // Quartz, and the Oracle format that follows it, require one of the two
      // day fields to be left open with ?
      if (day_rule<Traits>::value == day_field_rule::reject &&
          cex.dom_options.restricted && cex.dow_options.restricted)
         throw bad_cronexpr(
            "Specify a day of month or a day of week, and ? for the other");

      detail::set_cron_month<Traits>(fields[4], cex.months);

      if (fields.size() == 7)
         detail::year_field<Traits>::set(fields[6], cex.years);
      else
         detail::year_field<Traits>::set_all(cex.years); // no year field named

      // A date such as the 31st of February never arrives, so there is no
      // next occurrence to compute and the expression is rejected here rather
      // than leaving the caller to make sense of a search that never succeeds.
      if (!detail::has_reachable_date(cex.days_of_month, cex.months, cex.dom_options))
         throw bad_cronexpr("Date specified by the expression is invalid");

      cex.expr = expr;

      return cex;
   }

   template <typename Traits = cron_standard_traits>
   static std::tm cron_next(cronexpr const & cex, std::tm date)
   {
      if (cex.empty())
         throw bad_cronexpr("Invalid empty cron expression");

      std::time_t const original = utils::tm_to_time(date);
      if (INVALID_TIME == original) return {};

      std::tm result;
      if (INVALID_TIME == detail::find_next_after<Traits>(cex, original, result))
         return {};

      return result;
   }

   template <typename Traits = cron_standard_traits>
   static std::time_t cron_next(cronexpr const & cex, std::time_t const & date)
   {
      if (cex.empty())
         throw bad_cronexpr("Invalid empty cron expression");

      std::tm result;
      return detail::find_next_after<Traits>(cex, date, result);
   }

   // The time_point overloads keep the duration they were given, so a caller
   // working in milliseconds gets milliseconds back rather than the default
   // duration of the clock.
   //
   // When there is no next occurrence they return time_point::min(), which is
   // the counterpart of the INVALID_TIME the std::time_t overload returns.
   // Turning that sentinel into a time_point through from_time_t would name a
   // real instant just before the epoch, which reads as an ordinary answer.
   template <typename Traits = cron_standard_traits, typename Duration>
   static std::chrono::time_point<std::chrono::system_clock, Duration> cron_next(
      cronexpr const & cex,
      std::chrono::time_point<std::chrono::system_clock, Duration> const & time_point)
   {
      using result_type =
         std::chrono::time_point<std::chrono::system_clock, Duration>;

      auto const from = std::chrono::time_point_cast<
         std::chrono::system_clock::duration>(time_point);

      auto const next = cron_next<Traits>(
         cex, std::chrono::system_clock::to_time_t(from));

      if (INVALID_TIME == next) return (result_type::min)();

      return std::chrono::time_point_cast<Duration>(
         std::chrono::system_clock::from_time_t(next));
   }

   template <typename Traits = cron_standard_traits, typename Duration>
   static std::chrono::time_point<std::chrono::system_clock, Duration> cron_next_ceil(
      cronexpr const & cex,
      std::chrono::time_point<std::chrono::system_clock, Duration> const & time_point)
   {
      using result_type =
         std::chrono::time_point<std::chrono::system_clock, Duration>;

      auto const from = std::chrono::time_point_cast<
         std::chrono::system_clock::duration>(time_point);

      // std::chrono::system_clock::to_time_t truncates fractional seconds.
      // If the input time_point represents a known cron trigger time but is
      // slightly below that exact second, truncation can move it back one
      // second and cause cron_next to return the current trigger instead of
      // the next one.
      auto tt = std::chrono::system_clock::to_time_t(from);
      if (std::chrono::system_clock::from_time_t(tt) < from) {
         ++tt;
      }

      auto const next = cron_next<Traits>(cex, tt);

      if (INVALID_TIME == next) return (result_type::min)();

      return std::chrono::time_point_cast<Duration>(
         std::chrono::system_clock::from_time_t(next));
   }

   // The last time matching the expression that is strictly earlier than the
   // one given, the counterpart of cron_next.
   //
   // It is answered with repeated cron_next calls rather than a search running
   // backwards, so that it inherits the behaviour of the forward search rather
   // than restating it in mirror image. cron_next is monotonic, so "the next
   // occurrence is still earlier than the time asked about" holds for every
   // instant up to the answer and for none after it, and the boundary between
   // the two can be found by halving an interval.
   //
   // The interval starts as the smallest of a minute, an hour, a day, a month,
   // a year and the four year search horizon that contains an occurrence at
   // all. That costs one call per size tried, and bounds what the halving then
   // has to cover.
   //
   // The total is at most six probes, twenty seven halvings of the four year
   // interval and one final lookup, so thirty four calls; an expression firing
   // every second is answered in eight. The count follows the width of the
   // interval rather than the number of occurrences inside it, so an
   // expression firing every second of a single day costs no more than one
   // firing once a year.
   template <typename Traits = cron_standard_traits>
   static std::time_t cron_prev(cronexpr const & cex, std::time_t const & date)
   {
      if (cex.empty())
         throw bad_cronexpr("Invalid empty cron expression");

      static std::time_t const windows[] =
      {
         60,                   // a minute
         60 * 60,              // an hour
         24 * 60 * 60,         // a day
         31 * 24 * 60 * 60,    // a month
         366 * 24 * 60 * 60,   // a year
         4 * 366 * 24 * 60 * 60
      };

      std::time_t low = 0;
      bool bracketed = false;

      for (size_t i = 0; i < sizeof(windows) / sizeof(windows[0]); ++i)
      {
         // do not reach back past what a std::time_t can hold
         if (date < (std::numeric_limits<std::time_t>::min)() + windows[i]) continue;

         std::time_t const start = date - windows[i];
         std::time_t const first = cron_next<Traits>(cex, start);

         if (INVALID_TIME != first && first < date)
         {
            low = start;
            bracketed = true;
            break;
         }
      }

      if (!bracketed) return INVALID_TIME;

      // Narrow to the last instant whose next occurrence is still earlier than
      // the time asked about. The occurrence after that instant is the answer.
      std::time_t high = date;
      while (high - low > 1)
      {
         std::time_t const middle = low + (high - low) / 2;
         std::time_t const next = cron_next<Traits>(cex, middle);

         if (INVALID_TIME != next && next < date) low = middle;
         else                                     high = middle;
      }

      std::time_t const result = cron_next<Traits>(cex, low);

      return (INVALID_TIME != result && result < date) ? result : INVALID_TIME;
   }

   template <typename Traits = cron_standard_traits>
   static std::tm cron_prev(cronexpr const & cex, std::tm date)
   {
      std::time_t const original = utils::tm_to_time(date);
      if (INVALID_TIME == original) return {};

      std::time_t const result = cron_prev<Traits>(cex, original);
      if (INVALID_TIME == result) return {};

      std::tm out;
      if (utils::time_to_tm(&result, &out) == nullptr) return {};

      return out;
   }

   template <typename Traits = cron_standard_traits, typename Duration>
   static std::chrono::time_point<std::chrono::system_clock, Duration> cron_prev(
      cronexpr const & cex,
      std::chrono::time_point<std::chrono::system_clock, Duration> const & time_point)
   {
      using result_type =
         std::chrono::time_point<std::chrono::system_clock, Duration>;

      auto const from = std::chrono::time_point_cast<
         std::chrono::system_clock::duration>(time_point);

      // An occurrence at the truncated second is still earlier than a time
      // point carrying a fraction of one, so round the input up before asking.
      auto tt = std::chrono::system_clock::to_time_t(from);
      if (std::chrono::system_clock::from_time_t(tt) < from) {
         ++tt;
      }

      auto const previous = cron_prev<Traits>(cex, tt);

      if (INVALID_TIME == previous) return (result_type::min)();

      return std::chrono::time_point_cast<Duration>(
         std::chrono::system_clock::from_time_t(previous));
   }
}
