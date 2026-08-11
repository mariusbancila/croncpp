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
      std::string     expr;

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
         return
            seconds.none() ||
            minutes.none() ||
            hours.none() ||
            days_of_week.none() ||
            days_of_month.none() ||
            months.none();
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
         e1.months == e2.months;
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

      inline cron_int to_cron_int(CRONCPP_STRING_VIEW text)
      {
         if (text.empty())
            throw bad_cronexpr("Cron field value cannot be empty");

         for (auto const ch : text)
         {
            if (ch < '0' || ch > '9')
            {
               if (ch == 'L' || ch == 'W' || ch == '#')
                  throw bad_cronexpr(
                     std::string("Special character '") + ch + "' is not supported");

               throw bad_cronexpr(
                  "Invalid character in cron field: " + std::string(text));
            }
         }

         try
         {
            auto const value = std::stoul(std::string(text));
            if (value > static_cast<unsigned long>((std::numeric_limits<cron_int>::max)()))
               throw bad_cronexpr("Cron field value is out of range");

            return static_cast<cron_int>(value);
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

      static std::pair<cron_int, cron_int> make_range(
         CRONCPP_STRING_VIEW field,
         cron_int const minval,
         cron_int const maxval)
      {
         cron_int first = 0;
         cron_int last = 0;
         if (field.size() == 1 && field[0] == '*')
         {
            first = minval;
            last = maxval;
         }
         else if (!utils::contains(field, '-'))
         {
            first = to_cron_int(field);
            last = first;
         }
         else
         {
            auto parts = utils::split(field, '-');
            if (parts.size() != 2)
               throw bad_cronexpr("Specified range requires two fields");

            first = to_cron_int(parts[0]);
            last = to_cron_int(parts[1]);
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

      template <size_t N>
      static void set_cron_field(
         CRONCPP_STRING_VIEW value,
         std::bitset<N>& target,
         cron_int const minval,
         cron_int const maxval)
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
               auto[first, last] = detail::make_range(field, minval, maxval);
#else
               auto range = detail::make_range(field, minval, maxval);
               auto first = range.first;
               auto last = range.second;
#endif
               for (cron_int i = first - minval; i <= last - minval; ++i)
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
               auto[first, last] = detail::make_range(parts[0], minval, maxval);
#else
               auto range = detail::make_range(parts[0], minval, maxval);
               auto first = range.first;
               auto last = range.second;
#endif

               if (!utils::contains(parts[0], '-'))
               {
                  last = maxval;
               }

               auto delta = detail::to_cron_int(parts[1]);
               if(delta <= 0)
                  throw bad_cronexpr("Incrementer must be a positive value");

               for (cron_int i = first - minval; i <= last - minval; i += delta)
               {
                  target.set(i);
               }
            }
         }
      }

      template <typename Traits>
      static void set_cron_days_of_week(
         std::string value,
         std::bitset<7>& target)
      {
         auto days = utils::to_upper(value);
         auto days_replaced = detail::replace_ordinals(
            days,
#ifdef CRONCPP_IS_CPP17
            Traits::DAYS
#else
            Traits::DAYS()
#endif
         );

         if (days_replaced.size() == 1 && days_replaced[0] == '?')
            days_replaced[0] = '*';

         set_cron_field(
            days_replaced,
            target,
            Traits::CRON_MIN_DAYS_OF_WEEK,
            Traits::CRON_MAX_DAYS_OF_WEEK);
      }

      template <typename Traits>
      static void set_cron_days_of_month(
         std::string value,
         std::bitset<31>& target)
      {
         if (value.size() == 1 && value[0] == '?')
            value[0] = '*';

         set_cron_field(
            value,
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
         std::bitset<12> const & months)
      {
         static int const last_day_of[12] =
            { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

         for (size_t month = 0; month < months.size(); ++month)
         {
            if (!months.test(month)) continue;

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

      template <typename Traits>
      static size_t find_next_day(
         std::tm& date,
         std::bitset<31> const & days_of_month,
         size_t day_of_month,
         std::bitset<7> const & days_of_week,
         size_t day_of_week,
         std::bitset<7> const & marked_fields)
      {
         unsigned int count = 0;
         unsigned int maximum = 366;
         // day_of_week comes from std::tm::tm_wday and is always 0 (Sunday) to
         // 6 (Saturday), whatever the traits are. It is already the bit index,
         // because the cron value of Sunday is CRON_MIN_DAYS_OF_WEEK and the
         // bits are set at value - CRON_MIN_DAYS_OF_WEEK.
         while (
            (!days_of_month.test(day_of_month - Traits::CRON_MIN_DAYS_OF_MONTH) ||
            !days_of_week.test(day_of_week))
            && count++ < maximum)
         {
            add_to_field(date, cron_field::day_of_month, 1);

            day_of_month = date.tm_mday;
            day_of_week = date.tm_wday;

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

         unsigned int day_of_week = date.tm_wday;
         unsigned int day_of_month = date.tm_mday;
         auto updated_day_of_month = find_next_day<Traits>(
            date,
            cex.days_of_month,
            day_of_month,
            cex.days_of_week,
            day_of_week,
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
            if (date.tm_year - dot > Traits::CRON_MAX_YEARS_DIFF)
               return false;

            res = find_next<Traits>(cex, date, dot);
            if (!res) return res;
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
      if (fields.size() != 6)
         throw bad_cronexpr("cron expression must have six fields");

      detail::set_cron_field(fields[0], cex.seconds, Traits::CRON_MIN_SECONDS, Traits::CRON_MAX_SECONDS);
      detail::set_cron_field(fields[1], cex.minutes, Traits::CRON_MIN_MINUTES, Traits::CRON_MAX_MINUTES);
      detail::set_cron_field(fields[2], cex.hours, Traits::CRON_MIN_HOURS, Traits::CRON_MAX_HOURS);

      detail::set_cron_days_of_week<Traits>(fields[5], cex.days_of_week);

      detail::set_cron_days_of_month<Traits>(fields[3], cex.days_of_month);

      detail::set_cron_month<Traits>(fields[4], cex.months);

      // A date such as the 31st of February never arrives, so there is no
      // next occurrence to compute and the expression is rejected here rather
      // than leaving the caller to make sense of a search that never succeeds.
      if (!detail::has_reachable_date(cex.days_of_month, cex.months))
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

  template <typename Traits = cron_standard_traits>
  static std::chrono::system_clock::time_point cron_next(cronexpr const & cex, std::chrono::system_clock::time_point const & time_point) {
     return std::chrono::system_clock::from_time_t(cron_next<Traits>(cex, std::chrono::system_clock::to_time_t(time_point)));
  }

  template <typename Traits = cron_standard_traits>
  static std::chrono::system_clock::time_point cron_next_ceil(cronexpr const & cex, std::chrono::system_clock::time_point const & time_point) {
     // std::chrono::system_clock::to_time_t truncates fractional seconds.
     // If the input time_point represents a known cron trigger time but is
     // slightly below that exact second, truncation can move it back one
     // second and cause cron_next to return the current trigger instead of
     // the next one.
     auto tt = std::chrono::system_clock::to_time_t(time_point);
     if (std::chrono::system_clock::from_time_t(tt) < time_point) {
        ++tt;
     }
     return std::chrono::system_clock::from_time_t(cron_next<Traits>(cex, tt));
  }
}
