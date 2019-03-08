#include <string>

using namespace std;

// PARSE HTTP REQUESTS
#define GET_END "\r\n"      // end of GET request line
#define GET_STRING "GET /"  // start of GET request
#define FILENAME_INDEX 5    // capture file name from first header

// check complete request has been received
bool request_complete(string &req) {
    return ( req.find(GET_END) != string::npos );
}

// minimal syntax checking
bool bad_syntax(string &req) {
    return req.substr(0, FILENAME_INDEX) != GET_STRING ||
        req.find(' ', FILENAME_INDEX) == string::npos;
}

// extract file name to be served
// assumes: !bad_syntax(req)
string get_filename(string &req) {
    
    string name = req.substr(FILENAME_INDEX, 
        req.find(' ', FILENAME_INDEX) - FILENAME_INDEX);
    
    if ( name == "" )
        return "index.html";
    return name;
}
