#include <boost/utility/string_view.hpp>

namespace my
{
    template<typename Char>
    using basic_string_view = boost::basic_string_view<Char>;

    using string_view = basic_string_view<char>;
}
