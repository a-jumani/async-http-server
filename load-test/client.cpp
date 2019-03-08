#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <cstdlib>
#include <iostream>
#include <list>
#include <string>

using namespace std;
using boost::asio::ip::tcp;

size_t total_bytes;                             // track total bytes received from server
boost::mutex m;                                 // to control access to total_bytes

tcp::resolver::iterator endpoint_iterator;
boost::asio::io_service io_service;
string get_req;

void fetch_file(int requests) {

    size_t bytes_received = 0;
    boost::array<char, 128> buf;
    boost::system::error_code error;
    
    while ( requests-- ) {

        // connect with host
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);
        
        // send GET request
        socket.write_some(boost::asio::buffer(get_req));

        // receive reply
        for (;;) {
            size_t len = socket.read_some(boost::asio::buffer(buf), error);

            // connection closed cleanly by server
            if (error == boost::asio::error::eof)
                break;                            

            // throw other errors          
            else if (error)
                throw boost::system::system_error(error);
            
            // count bytes
            bytes_received += len;
        }
    }

    boost::lock_guard<boost::mutex> lg(m);
    total_bytes += bytes_received;
}

int main(int argc, char *argv[]) {

    try {

        if ( argc != 6 ) {
            cerr << "usage: ./client <host> <port> <threadpool> <total_requests> <filename>\n";
            return 1;
        }

        // parse input
        string hostname = argv[1];
        string port = argv[2];
        size_t num_threads = static_cast<size_t>(atoi(argv[3]));
        size_t reqs = static_cast<size_t>(atoi(argv[4])) / num_threads;
        string filename = argv[5];
        get_req = "GET /" + filename + " HTTP/1.0\r\n"
                  "Host: load-test-client\r\n"
                  "Accept: */*\r\n"
                  "\r\n";

        // resolve host name
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(hostname, port);
        endpoint_iterator = resolver.resolve(query);

        auto start = boost::chrono::system_clock::now();

        // create a pool of worker threads
        list<boost::thread> threads;
        for ( int i = 0; i < num_threads; ++i )
            threads.push_back(boost::thread(boost::bind(fetch_file, reqs)));
        
        // join the threads
        for ( auto it = threads.begin(); it != threads.end(); ++it )
            it->join();
        
        auto end = boost::chrono::system_clock::now();
        auto elapsed = boost::chrono::duration_cast<boost::chrono::milliseconds>(end - start);
        
        // report relevant statistics
        cout << "Total bytes received: " << total_bytes << endl;
        cout << "Total time: " << elapsed.count() << endl;

    } catch ( exception &e ) {
        cerr << e.what() << endl;
    }
    
    return 0;
}
