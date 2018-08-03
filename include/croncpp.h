#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <bitset>

namespace cron
{
   using cron_int  = uint8_t;

   struct bad_cronexpr : public std::exception
   {
   public:
      explicit bad_cronexpr(std::string_view message) :
         std::exception(message.data())
      {}
   };

   struct cronexpr
   {
      std::bitset<60> seconds;
      std::bitset<60> minutes;
      std::bitset<24> hours;
      std::bitset<7>  days_of_week;
      std::bitset<31> days_of_month;
      std::bitset<12> months;
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

      static std::vector<std::string> DAYS;
      static std::vector<std::string> MONTHS;
   };

   std::vector<std::string> cron_standard_traits::DAYS = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
   std::vector<std::string> cron_standard_traits::MONTHS = { "NIL", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

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

      static std::vector<std::string> DAYS;
      static std::vector<std::string> MONTHS;
   };

   std::vector<std::string> cron_oracle_traits::DAYS = { "NIL", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
   std::vector<std::string> cron_oracle_traits::MONTHS = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

   namespace detail
   {
      std::vector<std::string> split(std::string_view text, char const delimiter)
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

      constexpr bool contains(std::string_view text, char const ch) noexcept
      {
         return std::string_view::npos != text.find_first_of(ch);
      }

      cron_int to_cron_int(std::string_view text)
      {
         try
         {
            return static_cast<cron_int>(std::stoul(text.data()));
         }
         catch (std::exception const & ex)
         {
            throw bad_cronexpr(ex.what());
         }
      }

      std::string to_upper(std::string text)
      {
         std::transform(std::begin(text), std::end(text),
            std::begin(text), std::toupper);

         return text;
      }

      std::string replace_ordinals(std::string text, std::vector<std::string> const & replacement)
      {
         for (size_t i = 0; i < replacement.size(); ++i)
         {
            auto pos = text.find(replacement[i]);
            if (std::string::npos != pos)
               text.replace(pos, 3 ,std::to_string(i));
         }

         return text;
      }

      static std::pair<cron_int, cron_int> make_range(
         std::string_view field,
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
         else if (!contains(field, '-'))
         {
            first = to_cron_int(field);
            last = first;
         }
         else
         {
            auto parts = split(field, '-');
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

      template <int N>
      static void set_cron_field(
         std::string_view value,
         std::bitset<N>& target,
         cron_int const minval,
         cron_int const maxval)
      {
         auto fields = split(value, ',');
         if (fields.empty())
            throw bad_cronexpr("Expression parsing error");

         for (auto const & field : fields)
         {
            if (!contains(field, '/'))
            {
               auto[first, last] = detail::make_range(field, minval, maxval);
               for (cron_int i = first - minval; i <= last - minval; ++i)
               {
                  target.set(i);
               }
            }
            else 
            {
               auto parts = detail::split(field, '/');
               if (parts.size() != 2)
                  throw bad_cronexpr("Incrementer must have two fields");

               auto[first, last] = detail::make_range(parts[0], minval, maxval);

               if (!contains(parts[0], '-'))
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
         auto days = detail::to_upper(value);
         auto days_replaced = detail::replace_ordinals(days, Traits::DAYS);

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
         auto month = to_upper(value);
         auto month_replaced = replace_ordinals(month, Traits::MONTHS);

         set_cron_field(
            month_replaced, 
            target, 
            Traits::CRON_MIN_MONTHS,
            Traits::CRON_MAX_MONTHS);
      }
   }

   template <typename Traits = cron_standard_traits>
   cronexpr make_cron(std::string_view expr)
   {
      cronexpr cex;

      if (expr.empty())
         throw bad_cronexpr("Invalid empty cron expression");

      auto fields = detail::split(expr, ' ');
      fields.erase(
         std::remove_if(std::begin(fields), std::end(fields), 
            [](std::string_view s) {return s.empty(); }),
         std::end(fields));
      if (fields.size() != 6)
         throw bad_cronexpr("cron expression must have six fields");

      detail::set_cron_field(fields[0], cex.seconds, Traits::CRON_MIN_SECONDS, Traits::CRON_MAX_SECONDS);
      detail::set_cron_field(fields[1], cex.minutes, Traits::CRON_MIN_MINUTES, Traits::CRON_MAX_MINUTES);
      detail::set_cron_field(fields[2], cex.hours,   Traits::CRON_MIN_HOURS, Traits::CRON_MAX_HOURS);

      detail::set_cron_days_of_week<Traits>(fields[5], cex.days_of_week);

      detail::set_cron_days_of_month<Traits>(fields[3], cex.days_of_month);

      detail::set_cron_month<Traits>(fields[4], cex.months);

      return cex;
   }
}