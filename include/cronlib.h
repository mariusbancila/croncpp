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

   namespace detail
   {
      constexpr cron_int CRON_MAX_SECONDS = 60;
      constexpr cron_int CRON_MAX_MINUTES = 60;
      constexpr cron_int CRON_MAX_HOURS = 24;
      constexpr cron_int CRON_MAX_DAYS_OF_WEEK = 8;
      constexpr cron_int CRON_MAX_DAYS_OF_MONTH = 32;
      constexpr cron_int CRON_MAX_MONTHS = 12;
      constexpr cron_int CRON_MAX_YEARS_DIFF = 4;

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
            last = maxval - 1;
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

         if (first >= maxval || last >= maxval)
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
               for (cron_int i = first; i <= last; ++i)
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
                  last = maxval - 1;
               }

               auto delta = detail::to_cron_int(parts[1]);
               if(delta <= 0)
                  throw bad_cronexpr("Incrementer must be a positive value");

               for (cron_int i = first; i <= last; i += delta)
               {
                  target.set(i);
               }
            }
         }
      }
   }

   cronexpr make_cron(std::string_view expr)
   {
      cronexpr cex;

      if (expr.empty())
         throw bad_cronexpr("Invalid empty cron expression");

      auto fields = detail::split(expr, ' ');
      if (fields.size() != 6)
         throw bad_cronexpr("cron expression must have six fields");

      detail::set_cron_field(fields[0], cex.seconds, 0, detail::CRON_MAX_SECONDS);
      detail::set_cron_field(fields[1], cex.minutes, 0, detail::CRON_MAX_MINUTES);
      detail::set_cron_field(fields[2], cex.hours,   0, detail::CRON_MAX_HOURS);

      return cex;
   }
}