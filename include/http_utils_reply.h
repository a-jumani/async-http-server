#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

using namespace std;

// edit the following to customize responses
const string SERVE_PATH = "serve/";             // path to serve files from
const string E400_HTML = "Errors/400.html";     // file name in case of status 400
const string E404_HTML = "Errors/404.html";     // file name in case of status 404
const string SERVER_NAME = "Async-Http 1.0";

// HTTP syntax specifics
const string LINE_END = "\r\n";                 // end of a header
const string REQ_END = "\r\n\r\n";              // end of an HTTP request

#define FILE_READ_BUF 512                       // buffer to read files into

// header: status
namespace http_status {

    enum STATUS { OK, BAD_REQUEST, FILE_NOT_FOUND };

    const string ok = "HTTP/1.0 200 OK";
    const string bad_request = "HTTP/1.0 400 Bad Request";
    const string file_not_found = "HTTP/1.0 404 Not Found";
}

// parses out extension of a file
string get_extension(string filename) {
    size_t dot = filename.find('.');
    if ( dot != string::npos ) {
        return filename.substr(dot);
    }

    // did not find any extension
    return "";
}

// header: mime_type
string mime_type(string filename) {
    string ext = get_extension(filename);
    if( ext == ".htm" || ext == ".html" )
        return "text/html";
    if ( ext == ".gif" )
        return "image/gif";
    if ( ext == ".jpg" || ext == ".jpeg" )
        return "image/jpeg";
    if ( ext == ".png" )
        return "image/png";
    return "unknown/unknown";
}

// read contents of file into a string
bool read_file(string filename, string &resp) {
        
    boost::filesystem::ifstream ifs(SERVE_PATH + filename);
    
    if ( ifs ) {
        // read file into resp
        char buf[FILE_READ_BUF];
        while ( ifs.read(buf, sizeof(buf)).gcount() > 0 )
            resp.append(buf, ifs.gcount());
        
        // signal file was read successfully
        return true;
    }
    
    return false;
}

void form_reply(http_status::STATUS stat, string &headers, string &content, string filename="") {
    
    switch (stat) {

        case http_status::BAD_REQUEST:
            // create response body
            read_file(E400_HTML, content);
            content.append(LINE_END);
            
            // create response headers
            headers.append(http_status::ok + LINE_END);
            headers.append("Content-Type: " + mime_type(E400_HTML) + LINE_END);
            
            break;
        
        case http_status::FILE_NOT_FOUND:
            // create response body
            read_file(E404_HTML, content);
            content.append(LINE_END);

            // create response headers
            headers.append(http_status::file_not_found + LINE_END);
            headers.append("Content-Type: " + mime_type(E404_HTML) + LINE_END);
            
            break;
        
        case http_status::OK:
            // create response body
            if ( !read_file(filename, content) ) {
                form_reply(http_status::FILE_NOT_FOUND, headers, content);
                return;
            }
            content.append(LINE_END);
            
            // create response headers
            headers.append(http_status::ok + LINE_END);
            headers.append("Content-Type: " + mime_type(filename) + LINE_END);
            
            break;
    }

    headers.append("Server: " + SERVER_NAME + LINE_END);
    headers.append("Connection: close" + LINE_END);
    headers.append("Content-Length: " + 
        boost::lexical_cast<std::string>(content.size()) + LINE_END);
    headers.append("\n");
}
