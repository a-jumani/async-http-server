A light, simple web server that can be used to serve files in response to HTTP GET requests.

**Features:**
1. The implementation uses Boost libraries to be platform-independent.
2. It is asynchronous and uses Boost.Asio to achieve that.
3. The size of the threadpool can be specified at runtime.

**Testing:**
1. A platform-independent client using a threadpool for load testing.

**TODO:**
1. Introduce LRU caching using memory-mapped files to further boost performance.
2. Refactor the HTTP parsing and reply generation code from functional to object-oriented.
3. Support connection 'keep-alive' mode.