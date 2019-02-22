#ifndef ASYNC_HTTP_SERVER_HTTP_CONN_H_
#define ASYNC_HTTP_SERVER_HTTP_CONN_H_

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <string>

using boost::asio::ip::tcp;
using namespace std;

// TODO : factor out HTTP-specific code

#define MAX_CLIENT_MESSAGE_ALLOWED 20000
#define SOCK_READ_BUFFER 256

// HTTP specifics: GET request
#define GET_STRING "GET /"  // start of GET request
#define GET_END "\r\n"      // capture first header, i.e. GET /<file> HTTP
#define FILENAME_INDEX 5    // capture file name from first header

// TODO : complete the error messages
// HTTP specifics: responses
const string E400 = "HTTP/1.0 400 Bad Request\r\n"
                    "Content-Length: 1555\r\n"
                    "\n\r\n"
                    "<!DOCTYPE html><title>Error 400 (Bad Request)</title><body><p>Client issued a malformed or illegal request.</p></body>";
// const string E404 = "";
// const string E413 = "";


#define FILE_READ_BUF 512

// handle an http connection
// details: read and write to a socket is done asynchronously and
//   the socket is accessed through the shared pointer for easier
//   memory management
class http_conn : public boost::enable_shared_from_this<http_conn> {
public:
    
    typedef boost::shared_ptr<http_conn> http_conn_ptr;

    // create socket to accept a client into
    static http_conn_ptr create(boost::asio::io_service& io_service) {
        return http_conn_ptr(new http_conn(io_service));
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
    http_conn(boost::asio::io_service& io_service)
        : _socket(io_service), _data(nullptr) {}
    
    // read client request
    void read() {
        _socket.async_read_some(boost::asio::buffer(_rbuf),
            boost::bind(&http_conn::handle_read, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    // send error to the client
    void write_error(const string msg) {
        boost::asio::async_write(_socket, boost::asio::buffer(msg),
            boost::bind(&http_conn::handle_write, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    // send requested data to the client
    void write_data() {
        boost::asio::async_write(_socket, boost::asio::buffer(*_data),
            boost::bind(&http_conn::handle_write, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
        delete _data;
    }

    // report errors, if any, and close connection
    void handle_write(const boost::system::error_code& e, size_t len) {
        if ( e )
            cerr << "Error: " << e.message() << ", Client IP: " <<
                _socket.remote_endpoint().address() << "\n";

        _socket.close();
    }

    // 
    void handle_read(const boost::system::error_code& e, size_t len) {
        if ( !e ) {
            _message.append(_rbuf.data());

            // got the complete http request
            if ( _message.find(GET_END) != string::npos ) {

                if ( _message.substr(0, FILENAME_INDEX) != GET_STRING ||
                     _message.find(' ', FILENAME_INDEX) == string::npos )
                        write_error("Not a proper GET request\r\n\r\n");
                
                else {
                    // print GET request filename and client IP
                    string filename = string("serve/") + _message.substr(FILENAME_INDEX, 
                        _message.find(' ', FILENAME_INDEX) - FILENAME_INDEX);

                    cout << "GET request for " << filename << " from " <<
                        _socket.remote_endpoint().address() << "\n";
                    
                    if ( get_file(filename) )
                        write_data();
                    else
                        write_error("File not found\r\n\r\n");
                }
            
            // client is sending too much data
            } else if ( _message.size() > MAX_CLIENT_MESSAGE_ALLOWED ) {
                write_error("Not a proper GET request\r\n\r\n");
            
            // wait for more data from client
            } else {
                read();
            }
        
        } else {
            cerr << "Got error. Message: " << e.message() << ", Client IP: " <<
                _socket.remote_endpoint().address() << "\n";
        }
    }

    bool get_file(string filename) {
        
        ifstream ifs(filename);
        
        if (ifs) {
            
            _data = new string;
            char buf[FILE_READ_BUF];
            
            while ( ifs.read(buf, sizeof(buf)).gcount() > 0 )
                _data->append(buf, ifs.gcount());
            
            return true;
        }
        
        return false;
    }

    tcp::socket _socket;
    string _message;
    string *_data;
    boost::array<char, SOCK_READ_BUFFER> _rbuf;
};

#endif