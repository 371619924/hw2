
#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <set>
#include <cctype>

#include "httprequest.hpp"

// This is the heart of the HTTP parsing.

// You should first take the request string and break it up
// into substrings wherever there is a '\r\n' pair.  The 
// easiest way to do this is to iterate through the entire input,
// keeping track of the current character, where in the string you are
// and the last character. When you get a '\n' when the previous was '\r',
// you have found the end of the substring (well, 2 past the end 
// the substring), allowing you to use std::string's substr function.
//
// But at the same time you shouldn't ever have a '\n' without a prior '\r', or 
// a '\r' without a subsequent '\n', however this is something we are not going to
// require you to check for.
// 
// (Note that substr takes the starting index and the length, so you will
// want to keep track of the current index, the last start point, and possibly
// the length as well).

// Finally, if you have '\r\n\r\n' in a row that is the end of the headers and you
// can use a break to exit the loop.  This ending must be present and if it isn't you
// should raise a MalformedRequestException.  

// Anything past the \r\n\r\n is the "payload".

// Now that it is split up, you can then parse both the command (the first part)
// and then the headers.




// Trims leading and trailing whitespace from the string
std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t");// Find the location of first non-whitespace character
    size_t end = s.find_last_not_of(" \t");// Find the location of last non-whitespace character
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);// Return trimmed substring  start == std::string::npos means the state of not finding
}




HTTPRequest::HTTPRequest(std::string request) {
    // Find the location of end of the headers by searching for the double CRLF sequence
    size_t headerEnd = request.find("\r\n\r\n");// Find is &s (location)
    if (headerEnd == std::string::npos) {
        // If not found, throw a MalformedRequestException
        throw MalformedRequestException("fail");
    }

    // Parse the first line to get the command and resource
    std::string firstLine = request.substr(0, request.find("\r\n"));// Get the first line of the request  substr is extract not include \r\n
    if (firstLine.empty()) {

        throw MalformedRequestException("fail");
}

    // Now split the first line and ensure only one space between components
    size_t firstSpace = firstLine.find(' ');
    size_t secondSpace = firstLine.find(' ', firstSpace + 1); // Jump first space, go to next space
    size_t thirdSpace = firstLine.find(' ', secondSpace + 1); // same 

    // If there's no space or more than two spaces in the command line, throw an exception
    if (firstSpace == std::string::npos || secondSpace == std::string::npos || thirdSpace != std::string::npos) {
        throw MalformedRequestException("fail");
    }
    
    std::istringstream firstLineStream(firstLine);// Create a stream to parse the first line
    std::string method, resource, version;// Variables to hold the command, resource, and version

    // Read the first line
    firstLineStream >> method >> resource >> version;

    

    // Check that the version is HTTP/1.1
    if (version != "HTTP/1.1") {
        throw MalformedRequestException("fail");
    }
    // Ensure only one space between components
    if (method.empty() || resource.empty() || version.empty()) {
        throw MalformedRequestException("fail");
    }

    // Parse the command
    parse_command(method);

    

    

    // Set the resource
    resource = trim(resource);
    this->resource = resource;

    // Parse headers
    size_t start = firstLine.length() + 2; // Start after the command line and the first "\r\n"
    for (size_t i = start; i < headerEnd; ++i) {
        // Check for '\n' and the previous character was '\r'
        if (request[i] == '\n' && i > 0 && request[i - 1] == '\r') { // request[i] find the character that index is i
            // Extract the header line
            std::string header_line = request.substr(start, i - start - 1);

            if (header_line.empty()) {
                throw MalformedRequestException("fail");
            }

            std::shared_ptr<HTTPHeader> header = std::make_shared<HTTPHeader>(header_line);
            if (headers.find(header->get_name()) != headers.end()) {
                throw MalformedRequestException("fail");
        }
            headers[header->get_name()] = header; // Store header in the map

            // Update the start position for the next header
            start = i + 1;
        }

        // Note: 如果你有一个对象本身，直接使用点操作符 . 来访问它的成员。
        // 如果你有一个指针，你需要使用箭头操作符 -> 来访问指针指向的对象的成员
    }

    // Handle the last header line before "\r\n\r\n"
    if (start < headerEnd) {
        std::string header_line = request.substr(start, headerEnd - start);
        if (!header_line.empty()) {

        std::shared_ptr<HTTPHeader> header = std::make_shared<HTTPHeader>(header_line);
        if (headers.find(header->get_name()) != headers.end()) {

           throw MalformedRequestException("fail");
        }
        headers[header->get_name()] = header;
        
        }
    }

    // Extract the payload
    payload = request.substr(headerEnd + 4); // 4 accounts for the length of "\r\n\r\n"




    
}

    
const std::set<std::string> valid_commands = {"GET", "POST", "HEAD"};


void HTTPRequest::parse_command(std::string &command_string) {
    command_string = trim(command_string); // Trim whitespace

    // Check if the command is valid
    if (valid_commands.find(command_string) != valid_commands.end()) {
        command = command_string; // Store the valid command
    } else {
        throw MalformedRequestException("Invalid HTTP command: " + command_string);
    }
     
}

// This is the portion for a header.  The name should be the
// header name (the part before the :) in all lower case, and
// whitespace stripped out.  It technically should also not include
// any internal whitespace but we arent' checking for that.

// The value is ALL the data after the :, including any whitespace!


HTTPHeader::HTTPHeader(std::string s){
    size_t colon_pos = s.find(':');
    if (colon_pos == std::string::npos) {
        throw MalformedRequestException("fail");
    }

    // Extract the header name and trim whitespace
    name = trim(s.substr(0, colon_pos));
    std::transform(name.begin(), name.end(), name.begin(), ::tolower); // Convert to lowercase

    // Extract the header value (everything after the colon)
    value = s.substr(colon_pos + 1);
    value = trim(value); // Trim leading whitespace for the value
}