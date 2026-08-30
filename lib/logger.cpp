#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sstream>

#include "logger.h"

std::string ILogger::FormatMessage(const std::string& text, LogLevel msg_level) const{
    std::time_t t = std::time(nullptr);
    std::tm* tm_info = std::localtime(&t);

    std::string level_str;
    switch(msg_level){
        case LogLevel::INFO: level_str = "INFO"; break;
        case LogLevel::WARNING: level_str = "WARNING"; break;
        case LogLevel::ERROR: level_str = "ERROR"; break;
    }

    std::stringstream ss;
    ss << "[" << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S") << "] "
            << "[" << level_str << "] " 
            << text << std::endl;
    return ss.str();
}

FileLogger::FileLogger(const std::string& filename_input, LogLevel default_level) :
    filename(filename_input), level(default_level){
        file_stream.open(filename_input, std::ios::app);
}

FileLogger::~FileLogger(){
    file_stream.close();
}

void FileLogger::SetLevel(LogLevel new_level){
    level = new_level;
}

LogLevel FileLogger::GetLevel() const{
    return level;
}

void FileLogger::Log(const std::string& text, LogLevel msg_level){
    if (msg_level >= level){
        std::string t = FormatMessage(text, msg_level);
        file_stream << t << std::flush;
    }
}

SocketLogger::SocketLogger(const std::string& ip, int port, LogLevel default_level)
    : level(default_level), ip_address(ip), descriptor(-1){

    descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptor < 0){
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0){
        std::cerr << "Invalid address / Address not supported" << std::endl;
        close(descriptor);
        descriptor = -1;
        return;
    }

    if (connect(descriptor, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0){
        std::cerr << "Connection failed" << std::endl;
        close(descriptor);
        descriptor = -1;
    }
}

SocketLogger::~SocketLogger(){
    if (descriptor >= 0){
        close(descriptor);
    }
}


void SocketLogger::SetLevel(LogLevel new_level){
    level = new_level;
}

LogLevel SocketLogger::GetLevel() const{
    return level;
}

void SocketLogger::Log(const std::string& text, LogLevel msg_level){
    if (msg_level >= level){
        std::string final_msg = FormatMessage(text, msg_level);
        send(descriptor, final_msg.c_str(), final_msg.length(), 0);
    }
}
