#include "my_sv.hpp"

my::string_view temporary() {
    return "abc";
}

int main()
{
    my::string_view sv{"abc"};

    sv.to_string();

    temporary().to_string();

    auto a{ sv.to_string() };
    auto b{temporary().to_string()};

    my::string_view* sv_ptr = &sv;

    auto c{sv_ptr->to_string()};

    auto d {(*sv_ptr).to_string()};

    auto e{ sv_ptr->substr(2).to_string()};
}
