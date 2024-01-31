#include <boost/asio.hpp>

#include <array>
#include <iostream>
#include <string_view>

int main(int argc, char** argv)
{
	boost::asio::io_context io;

    boost::asio::random_access_file file{io, argv[1], boost::asio::random_access_file::read_only};

    std::array<char, 10> buffer;
    size_t const read{file.read_some_at(0, boost::asio::buffer(buffer))};

    std::cout << std::string_view(buffer.data(), read) << '\n';
}
