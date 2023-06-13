#include <iostream>
#include <lexy/action/validate.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl/digit.hpp>
#include <lexy/dsl/punctuator.hpp>
#include <lexy/error.hpp>
#include <string>
#include <vector>

#include <lexy/encoding.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/action/parse.hpp>
#include <lexy/dsl.hpp>

#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

namespace util
{
    struct error
    {
        std::size_t line_number;
        std::size_t column_number;

        std::string_view production;
    };

    struct error_callback
    {
        using return_type = error;

        template <typename Input, typename Tag>
        error operator()(lexy::error_context<Input> const& context, lexy::error_for<Input, Tag> const& error)
        {
            lexy::input_location const context_location{lexy::get_input_location(context.input(), context.position())};
            lexy::input_location const location{lexy::get_input_location(context.input(), error.position(), context_location.anchor())};

            std::cout << "Context: " << context.input().size() << ' ' << context.position() << ' ' << context.production() << ' ' << location.line_nr() << ' ' << location.column_nr() << '\n';

            if constexpr (std::is_same_v<Tag, lexy::expected_literal>)
            {
                //The error can be raised while trying to parse string() with length() beginning at position(); the code unit at std::next(position(), index()) did not match character(). 
                //If the remaining input contained fewer code units than string(), index() indicates the first code unit that was missing.
                std::cout << "Error expected literal: " << error.string() << ' ' << error.length() << ' ' << error.position() << ' ' << error.index() << '\n';
            }
            else if constexpr (std::is_same_v<Tag, lexy::expected_keyword>)
            {
                //The error can be raised while trying to parse the keyword string() with length() against the identifier [begin(), end()), but it was a different identifier. The position() is also begin().
                std::cout << "Error expected keyword: " << error.string() << ' ' << error.length() << ' ' << error.position() << '\n';
            }
            else if constexpr (std::is_same_v<Tag, lexy::expected_char_class>)
            {
                //The error can be raised while trying to parse the character class with the human-readable name character_class() at position(). This happens in rules like lexy::dsl::ascii or lexy::dsl::digit.
                std::cout << "Expected character class: " << error.name() << '\n';
            }
            else 
            {
                std::cout << "Generic error: " << error.message() << '\n';
            }
            
            return {location.line_nr() - 1, location.column_nr() - 1, context.production()};
        }
    };
}

namespace grammar
{

struct number
{
    static constexpr auto whitespace = dsl::unicode::space;
    static constexpr auto rule = dsl::hash_sign + dsl::integer<std::size_t>(dsl::n_digits<3>) + dsl::eof;
    static constexpr auto value = lexy::as_integer<std::size_t>;
};

}

int main()
{
    std::string line;
    while (std::getline(std::cin, line))
    {
        lexy::string_input<lexy::utf8_char_encoding> input{line};
        
        if (auto result{lexy::parse<grammar::number>(input, lexy::collect<std::vector<util::error>>(util::error_callback{}))})
        {
            std::cout << '"' << line << "\" evaluates to " << result.value() << '\n';
        }
        else
        {
            for (auto const& error : result.errors())
            {
                std::cout << "Parsing \"" << line << "\" failed during production " << error.production << " failed on line " << error.line_number << " and column " << error.column_number << '\n';
            }
        }
    }
}
