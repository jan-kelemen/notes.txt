#include <boost/utility/string_view.hpp>

boost::string_view temporary() {
    return "abc";
}

int main()
{
    boost::string_view sv{"abc"};

    sv.to_string();

    temporary().to_string();

    auto a{ sv.to_string() };
    auto b{temporary().to_string()};

    boost::string_view* sv_ptr = &sv;

    auto c{sv_ptr->to_string()};

    auto d {(*sv_ptr).to_string()};
}
