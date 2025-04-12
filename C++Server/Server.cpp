#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <json11.hpp>

using boost::asio::ip::tcp;
using json11::Json;

class MCPServer
{
public:
    MCPServer(boost::asio::io_context& io_service, short port)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
        socket_(io_service)
    {
        start_accept();
    }

private:
    void start_accept()
    {
        acceptor_.async_accept(socket_, boost::bind(&MCPServer::handle_accept, this, boost::asio::placeholders::error));
    }

    void handle_accept(const boost::system::error_code& error)
    {
        if (!error)
        {
            std::cout << "Client connected!" << std::endl;
            start_read();
        }
        else
        {
            std::cerr << "Error accepting client connection: " << error.message() << std::endl;
        }
    }

    void start_read()
    {
        std::cout << "Starting to read data..." << std::endl;
        // Check if data is available
        if (socket_.available() > 0) {
            std::cout << "Data available to read!" << std::endl;
            std::cout << "Available bytes: " << socket_.available() << std::endl;
            std::vector<char> peek_buf(100);
            size_t peek_len = socket_.receive(boost::asio::buffer(peek_buf), boost::asio::socket_base::message_peek);
            std::cout << "Peeked data: ";
            for (size_t i = 0; i < peek_len; ++i) {
                std::cout << peek_buf[i];
            }
            std::cout << std::endl;
        }
        boost::asio::async_read_until(socket_, buffer_, "\n", boost::bind(&MCPServer::handle_read, this, boost::asio::placeholders::error));
    }

    void handle_read(const boost::system::error_code& error)
    {
        std::cout << "Read Data Over Handing..." << std::endl;
        if (!error)
        {
            std::istream is(&buffer_);
            std::string json_str;
            std::getline(is, json_str);

            // 修复关键问题：清理 Windows 换行
            if (!json_str.empty() && json_str.back() == '\r') {
                json_str.pop_back();
            }

            std::cout << "Raw message: " << json_str << std::endl;

            if (json_str == "ping") {
                Json response = Json::object{
                    {"status", "success"},
                    {"result", Json::object{{"message", "pong"}}}
                };
                send_response(response);
                return;
            }

            std::string err;
            Json request = Json::parse(json_str, err);

            if (err.empty())
            {
                std::cout << "Received request: " << request.dump() << std::endl;
                Json response = handle_request(request);
                send_response(response);
            }
            else
            {
                std::cerr << "Invalid JSON: " << err << std::endl;
                Json error_response = Json::object{
                    {"status", "error"},
                    {"message", "Invalid JSON"}
                };
                send_response(error_response);
            }

            start_read();
        }
        else
        {
            std::cerr << "Error reading from client: " << error.message() << std::endl;
        }
    }


    Json handle_request(const Json& request)
    {
        std::string command_type = request["type"].string_value();
        Json params = request["params"];

        std::cout << "Command Type: " << command_type << std::endl;
        std::cout << "Params: " << params.dump() << std::endl;

        if (command_type == "add_cube") {
            std::cout << std::endl;
            std::cout << "SUCCESS_FUNCTION" << std::endl;
            std::cout << "Function: add_cube" << std::endl;
            std::cout << "END_FUNCTION" << std::endl;
            std::cout << std::endl;
        }

        return Json::object{
            {"status", "success"},
            {"result", Json::object{
                {"message", "Command received"},
                {"echo_type", command_type},
                {"echo_params", params}
            }}
        };
    }

    void send_response(const Json& response)
    {
        auto response_str = std::make_shared<std::string>(response.dump() + "\n");
        boost::asio::async_write(socket_, boost::asio::buffer(*response_str),
            [this, response_str](const boost::system::error_code& error, std::size_t /*bytes_transferred*/)
            {
                handle_write(error);
            });
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (error)
        {
            std::cerr << "Error writing to client: " << error.message() << std::endl;
        }
        else
        {
            // 写入成功后启动下一次读取
            start_read();
        }
    }

private:
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    boost::asio::streambuf buffer_;
};

int main()
{
    try
    {
        boost::asio::io_context io_service;
        MCPServer server(io_service, 6400);
        io_service.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
