#include <cstdint>

#include <boost/program_options.hpp>

#include <flatbuffers/flatbuffers.h>
#include <wire_generated.h>

#include <zmq.hpp>
#include <fmt/core.h>
#include <date/date.h>

#include <lexy/action/parse.hpp>     // lexy::parse
#include <lexy/callback.hpp>         // value callbacks
#include <lexy/dsl.hpp>              // lexy::dsl::*
#include <lexy/input/argv_input.hpp> // lexy::argv_input

#include <lexy_ext/report_error.hpp> // lexy_ext::report_error

#include <iostream>

namespace
{
namespace ip
{
    // Stores an IP address.
    struct ip_address
    {
        int           version; // 4 or 6
        std::uint16_t pieces[8];
    };

    // Constructs an IPv4 address.
    constexpr ip_address ipv4(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d)
    {
        ip_address result{4, {}};
        result.pieces[0] = static_cast<std::uint16_t>((a << 8) | b);
        result.pieces[1] = static_cast<std::uint16_t>((c << 8) | d);
        return result;
    }

    // Constructs an IPv6 address.
    class ipv6_builder
    {
    public:
        constexpr ipv6_builder() : _pieces{}, _count(0), _elision_index(-1) {}

        constexpr int count() const
        {
            return _count;
        }
        constexpr bool has_elision() const
        {
            return _elision_index >= 0;
        }

        constexpr bool elision()
        {
            if (has_elision())
                return false;

            _elision_index = _count;
            return true;
        }
        constexpr void piece(std::uint16_t p)
        {
            if (_count < 8)
                _pieces[_count] = p;
            ++_count;
        }
        constexpr void ipv4(ip_address ip)
        {
            LEXY_PRECONDITION(ip.version == 4);
            if (_count <= 6)
            {
                _pieces[_count]     = ip.pieces[0];
                _pieces[_count + 1] = ip.pieces[1];
            }
            _count += 2;
        }

        constexpr ip_address finish() &&
        {
            ip_address result{6, {}};

            auto dst = 0;
            auto src = 0;

            // Copy everything before the elision.
            while (src < _elision_index)
            {
                result.pieces[dst] = _pieces[src];
                ++dst;
                ++src;
            }

            // Skip over the zeroes.
            auto zero_count = has_elision() ? 8 - _count : 0;
            dst += zero_count;

            // Copy everything after the elision.
            while (src < _count && src < 8)
            {
                result.pieces[dst] = _pieces[src];
                ++dst;
                ++src;
            }

            return result;
        }

    private:
        std::uint16_t _pieces[8];
        int           _count;
        int           _elision_index;
    };
} // namespace ip

// Formal specification: https://tools.ietf.org/html/draft-main-ipaddr-text-rep-00#section-3
namespace grammar
{
    namespace dsl = lexy::dsl;

    // d8 in the specification.
    struct ipv4_octet
    {
        static constexpr auto rule = [] {
            auto digits = dsl::digits<>.no_leading_zero();
            return dsl::integer<std::uint8_t>(digits);
        }();

        static constexpr auto value = lexy::as_integer<std::uint8_t>;
    };

    // Ipv4address in the specification.
    struct ipv4_address
    {
        static constexpr auto rule  = dsl::times<4>(dsl::p<ipv4_octet>, dsl::sep(dsl::period));
        static constexpr auto value = lexy::callback<ip::ip_address>(&ip::ipv4);
    };

    // If lookahead finds a period after the digits, it must be an IPv4 address.
    constexpr auto ipv4_address_condition = dsl::peek(dsl::digits<> + dsl::period);

    // h16 in the specification.
    struct ipv6_piece
    {
        static constexpr auto rule  = dsl::integer<std::uint16_t, dsl::hex>;
        static constexpr auto value = lexy::as_integer<std::uint16_t>;
    };

    // IPv6address in the specification.
    // We can't easily parse it using the DSL, so we provide a scan function.
    struct ipv6_address : lexy::scan_production<ip::ip_address>
    {
        struct missing_pieces
        {
            static constexpr auto name = "not enough IPv6 pieces";
        };
        struct too_many_pieces
        {
            static constexpr auto name = "too many IPv6 pieces";
        };

        struct duplicate_elision
        {
            static constexpr auto name = "duplicate zero elision";
        };

        // Called by the algorithm to perform the actual parsing.
        // The scanner manages the input and allows dispatching to other rules.
        // (scan_result is a typedef injected by `lexy::scan_production`).
        template <typename Reader, typename Context>
        static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>& scanner)
        {
            ip::ipv6_builder builder;

            // We parse arbitrary many pieces in a loop.
            while (true)
            {
                // At any point, we can have zero elision with a double colon.
                if (auto elision_begin = scanner.position(); scanner.branch(dsl::double_colon))
                {
                    if (!builder.elision())
                        // Report an error if we had an elision already.
                        // We trivially recover from it and continue parsing.
                        scanner.error(duplicate_elision{}, elision_begin, scanner.position());

                    // Check whether it is followed by another piece, as it is allowed to be at the
                    // end.
                    if (!scanner.peek(dsl::digit<dsl::hex>))
                        break;
                }
                // A normal separator is only allowed if we had a piece already.
                else if (builder.count() > 0 && !scanner.branch(dsl::colon))
                {
                    // If we don't have a separator, we exit the loop.
                    break;
                }

                // A piece is either an IPv4 address.
                if (scanner.branch(ipv4_address_condition))
                {
                    auto ipv4 = scanner.parse(ipv4_address{});
                    if (!scanner)
                        return lexy::scan_failed;
                    builder.ipv4(ipv4.value());

                    // If it was an IPv4 address, nothing must follow it.
                    break;
                }
                else
                {
                    // Or hex digits.
                    auto piece = scanner.parse(ipv6_piece{});
                    if (!scanner)
                        return lexy::scan_failed;
                    builder.piece(piece.value());
                }
            }

            // Check that we're having the correct amount of pieces.
            // Report an error otherwise, but trivially recover from it.
            if (builder.count() < 8 && !builder.has_elision())
                scanner.error(missing_pieces{}, scanner.begin(), scanner.position());
            else if (builder.count() > 8 || (builder.has_elision() && builder.count() == 8))
                scanner.error(too_many_pieces{}, scanner.begin(), scanner.position());

            // And return our result.
            return LEXY_MOV(builder).finish();
        }
    };

    // Either IPv4 or IPv6.
    struct ip_address
    {
        static constexpr auto rule = [] {
            auto ipv4 = ipv4_address_condition >> dsl::p<ipv4_address>;
            auto ipv6 = dsl::else_ >> dsl::p<ipv6_address>;

            return (ipv4 | ipv6) + dsl::try_(dsl::eof);
        }();

        static constexpr auto value = lexy::forward<ip::ip_address>;
    };
} // namespace grammar
} // namespace

struct timestamp
{
    int16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint16_t milliseconds;
};

timestamp current_timestamp() noexcept
{
    using namespace date;
    std::chrono::system_clock::time_point const time =
        std::chrono::system_clock::now();

    auto const daypoint = floor<days>(time);
    year_month_day const ymd = year_month_day(daypoint);

    auto const tmd = make_time(time - daypoint);

    std::chrono::duration tp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            time.time_since_epoch());
    tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);

    return {static_cast<int16_t>(static_cast<int>(ymd.year())),
        static_cast<uint8_t>(static_cast<unsigned>(ymd.month())),
        static_cast<uint8_t>(static_cast<unsigned>(ymd.day())),
        static_cast<uint8_t>(tmd.hours().count()),
        static_cast<uint8_t>(tmd.minutes().count()),
        static_cast<uint8_t>(tmd.seconds().count()),
        static_cast<uint16_t>(tp / std::chrono::milliseconds(1))};
}

template<>
struct fmt::formatter<timestamp>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(timestamp const& t, FormatContext& ctx)
    {
        return format_to(ctx.out(),
            "{:04}-{:02}-{:02}T{:02}:{:02}:{:02}:{:03}",
            t.year,
            t.month,
            t.day,
            t.hours,
            t.minutes,
            t.seconds,
            t.milliseconds);
    }
};

std::optional<bool> parse_options(int argc, char** argv)
{
    namespace bpo = boost::program_options;

    // https://stackoverflow.com/a/14940678
    try
    {
        bpo::options_description cmd_options{"Allowed options"};
        cmd_options.add_options()("help", "produce help message");
        bpo::variables_map vm;
        bpo::store(bpo::parse_command_line(argc, argv, cmd_options), vm);

        if (vm.count("help"))
        {
            std::cout << cmd_options << '\n';
            return std::nullopt;
        }

        bpo::notify(vm);

        return {true};
    }
    catch (const bpo::error& ex)
    {
        std::cerr << "Failed start with given command line arguments: "
                  << ex.what() << '\n';
        return std::nullopt;
    }
}

int main(int argc, char* argv[])
{
    parse_options(argc, argv);

    flatbuffers::FlatBufferBuilder raw_builder;

    auto const query_off = CreateQueryDirect(raw_builder, fmt::format("{}", current_timestamp()).c_str());

    // Scan the IP address provided at the commandline.
    lexy::argv_input input(argc, argv);
    auto             result = lexy::parse<grammar::ip_address>(input, lexy_ext::report_error);
    if (!result.has_value())
        return 1;

    auto value = result.value();

    if (value.version == 4)
        fmt::println("{} 0x{:0x}{:0x}", current_timestamp(), value.pieces[0], value.pieces[1]);
    else
        fmt::println("{} 0x{:0x}{:0x}{:0x}{:0x}{:0x}{:0x}{:0x}{:0x}",
                current_timestamp(),
                value.pieces[0],
                value.pieces[1],
                value.pieces[2],
                value.pieces[3],
                value.pieces[4],
                value.pieces[5],
                value.pieces[6],
                value.pieces[7]);

    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::push);
    sock.bind("inproc://test");
    sock.send(zmq::str_buffer("Hello, world"), zmq::send_flags::dontwait);

    return result ? 0 : 1;
}

