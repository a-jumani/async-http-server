#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <cstdlib>
#include <iostream>
#include "http_server.h"

#define MAX_THREADPOOL_SIZE_ALLOWED 50

using namespace std;

int main(int argc, char *argv[]) {

    try {

        if ( argc != 3 ) {
            cerr << "usage: ./server <port> <threadpool size>\n";
            return 1;
        }

        int port = atoi(argv[1]);
        size_t tp_size = static_cast<size_t>(atoi(argv[2]));

        // check threadpool size asked for is reasonable
        if ( tp_size > MAX_THREADPOOL_SIZE_ALLOWED ) {
            cerr << "Error: chosen threadpool size is too big. Max allowed is "
                << MAX_THREADPOOL_SIZE_ALLOWED << ".\n";
            return 1;
        }

        // launch io service
        boost::asio::io_service io_service;

        // create a threadpool
        boost::thread_group pool;
        boost::asio::io_service::work work(io_service);

        // launch threads
        for ( size_t i = 0; i < tp_size; ++i )
            pool.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
        
        // launch the server
        http_server server(io_service, port);
        io_service.run();

    } catch ( exception &e ) {
        cerr << e.what() << endl;
    }
    
    return 0;
}