#ifndef ASYNC_HTTP_SERVER_HTTP_SERVER_H_
#define ASYNC_HTTP_SERVER_HTTP_SERVER_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "http_conn.h"

using boost::asio::ip::tcp;

// run an http server on the specified port
// details: the server accepts incoming connections asynchronously. These connections are
//   then sent off to a threadpool to be serviced.
class http_server {
public:
    // creates a socket and starts listening for connections
    http_server(boost::asio::io_service& io_service, short port)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
            start_accept();
    }

private:
    // asynchronously accept incoming client connection
    void start_accept() {
        auto new_connection = http_conn::create(acceptor_.get_io_service());
        
        acceptor_.async_accept(new_connection->socket(),
            boost::bind(&http_server::handle_accept, this, new_connection,
            boost::asio::placeholders::error));
    }

    // handle accepted client connection
    void handle_accept(http_conn::http_conn_ptr new_connection,
        const boost::system::error_code& error) {
        
        // send this task to a pool of threads
        if (!error) {
            // new_connection->start();
            acceptor_.get_io_service().post(boost::bind(&http_conn::start, new_connection));
        }

        start_accept();
    }

    tcp::acceptor acceptor_;
};

#endif