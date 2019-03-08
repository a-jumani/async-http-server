#ifndef ASYNC_HTTP_SERVER_HTTP_SERVER_H_
#define ASYNC_HTTP_SERVER_HTTP_SERVER_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "http_conn.h"

using boost::asio::ip::tcp;

// run an http server
// details: the server accepts incoming connections asynchronously.
//   Boss-workers pattern is employed here, where the workers are
//   expected to be bound to the io_service running this server.
class http_server {
public:
    // creates a listener
    http_server(boost::asio::io_service& io_service, short port)
        : _acceptor(io_service, tcp::endpoint(tcp::v4(), port)) {
            start_accept();
    }

private:
    // accept incoming connections
    void start_accept() {
        // asynchronously accept new clients in to the http_conn object
        auto new_connection = http_conn::create(_acceptor.get_io_service(), &_log);
        _acceptor.async_accept(new_connection->socket(),
            boost::bind(&http_server::handle_accept, this, new_connection,
            boost::asio::placeholders::error));
    }

    // handle accepted client connection
    void handle_accept(http_conn::http_conn_ptr new_connection,
        const boost::system::error_code& error) {
        
        // send off client handling to the workers
        if (!error)
            _acceptor.get_io_service().post(boost::bind(&http_conn::start, new_connection));
        
        start_accept();
    }

    tcp::acceptor _acceptor;        // to accept connections
    boost::mutex _log;              // to serialize access to stdout and stderr
};

#endif