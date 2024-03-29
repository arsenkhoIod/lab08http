#include <suggester_client.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

#include "suggester_client.hpp"

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

// Performs an HTTP GET and prints the response
int main(int argc, char** argv)
{
  try
  {
    // Check command line arguments.
    if(argc != 4 && argc != 5)
    {
      std::cerr <<
                "Usage: http-client-sync <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
                "Example:\n" <<
                "    http-client-sync www.example.com 80 /\n" <<
                "    http-client-sync www.example.com 80 / 1.0\n";
      return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const target = argv[3];
    int version = argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

    // The io_context is required for all I/O
    net::io_context ioc;

    // подключение к серверу
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    suggester_client client;
    try {
      while (true){
        // отправляем запрос серверу
        http::request<http::string_body> req{http::verb::post, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        //обрабатывает сообщение, например "hello"
        req.body() = client.request();
        req.prepare_payload();
        // непосредственно отправляет запрос - write
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::string_body> res;

        // непосредственно чтение - read
        http::read(stream, buffer, res);
        if (res.result() == http::status::bad_request){
          std::cout << res.body();
          //подобие break, только continue переходит к следующей итерации
          continue;
        }
        // Write the message to standard out
        try {
          client.parse_suggest(res.body(), std::cout);
        } catch (const std::runtime_error& e) {
          std::cout << e.what();
        }
      }
    } catch (const beast::system_error & e) {
      std::cout << e.what();
    }

    // Gracefully close the socket
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if(ec && ec != beast::errc::not_connected)
      throw beast::system_error{ec};

    // If we get here then the connection is closed gracefully
  }
  catch(std::exception const& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}