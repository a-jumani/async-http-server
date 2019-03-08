#ifndef ASYNC_HTTP_SERVER_HTTP_CONN_H_
#define ASYNC_HTTP_SERVER_HTTP_CONN_H_

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <string>
#include "http_utils_reply.h"
#include "http_utils_request.h"

using boost::asio::ip::tcp;
using namespace std;

#define MAX_CLIENT_REQ_ALLOWED 20000
#define SOCK_READ_BUFFER 1500

// handle an http connection
// details: reads and write to a socket are done asynchronously and
//   the socket is accessed through the shared pointer for easier
//   memory management.
class http_conn : public boost::enable_shared_from_this<http_conn> {
public:
    
    typedef boost::shared_ptr<http_conn> http_conn_ptr;

    // create socket to accept a client into
    static http_conn_ptr create(boost::asio::io_service &io_service,
        boost::mutex *log_lock) {
            return http_conn_ptr(new http_conn(io_service, log_lock));
    }

    // getter for _socket
    tcp::socket& socket() {
        return _socket;
    }

    // start serving the client
    void start() {
        read();
    }

private:
    http_conn(boost::asio::io_service& io_service, boost::mutex *log_lock)
        : _socket(io_service), _log(log_lock) {}
    
    // read client request
    void read() {
        _socket.async_read_some(boost::asio::buffer(_rbuf),
            boost::bind(&http_conn::handle_read, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    // send reply to the client
    void write() {
        vector<boost::asio::const_buffer> bufs;
        bufs.push_back(boost::asio::buffer(_headers));
        bufs.push_back(boost::asio::buffer(_content));

        boost::asio::async_write(_socket, bufs,
            boost::bind(&http_conn::handle_write, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    // log errors, if any, and close connection
    void handle_write(const boost::system::error_code& e, size_t len) {
        if ( e ) {
            boost::lock_guard<boost::mutex> lg(*_log);
            cerr << "Error: " << e.message() << ", Client IP: " <<
                _socket.remote_endpoint().address() << "\n";
        }
        
        _socket.shutdown(_socket.shutdown_both);
    }

    // process received data
    void handle_read(const boost::system::error_code& e, size_t len) {
        if ( !e ) {
            
            // read client data from the buffer
            _request.append(_rbuf.begin(), _rbuf.begin() + len);

            // client request is complete
            if ( request_complete(_request) ) {

                // create reply for malformed syntax
                if ( bad_syntax(_request) ) {
                    form_reply(http_status::BAD_REQUEST, _headers, _content);
                
                // create reply for a proper request
                } else {
                    string filename = get_filename(_request);
                    form_reply(http_status::OK, _headers, _content, filename);
                    
                    // log request
                    boost::lock_guard<boost::mutex> lg(*_log);
                    cout << "GET request for " << filename << " from " <<
                        _socket.remote_endpoint().address() << "\n";
                }

                write();
            
            // client is sending too much data
            } else if ( _request.size() > MAX_CLIENT_REQ_ALLOWED ) {
                form_reply(http_status::BAD_REQUEST, _headers, _content);
                write();
            
            // wait for more data from client
            } else {
                read();
            }
        
        } else {
            // log error
            boost::lock_guard<boost::mutex> lg(*_log);
            cerr << "Error: " << e.message() << ", Client IP: " <<
                _socket.remote_endpoint().address() << "\n";
        }
    }

    tcp::socket _socket;                            // client socket
    boost::mutex *_log;                             // to serialize access to stdout and stderr
    string _request;                                // client request
    boost::array<char, SOCK_READ_BUFFER> _rbuf;     // buffer to read client request into
    string _headers;                                // reply headers
    string _content;                                // reply content
};

#endif