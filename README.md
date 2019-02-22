The small, simple web server that can be used to serve files in response to HTTP GET requests.

**Features:**
1. The implementation is uses Boost libraries to be platform-independent.
2. It is asynchronous and uses Boost.Asio to achieve that.
3. The size of the threadpool can be specified at runtime.

**TODO:**
1. Support for GET requests needs to be extended. It is rudimentary at the moment. Hence, only `curl` and the `load-test/client.cpp` work for now.
2. Caching is needed to further boost performance.