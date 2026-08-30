#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <iostream>
#include <mutex>
#include <thread>

#include "logger.h"

struct LogTask{
    std::string text;
    LogLevel level;
};

std::queue<LogTask> task_queue;
std::mutex queue_mutex;
std::condition_variable queue_cv;
std::atomic<bool> is_running{true};

bool ParseArgs(int argc, char** argv, std::string& filename, LogLevel& lvl){
    if (argc != 3){
        std::cerr <<
        "Invalid number of arguments. Please enter a file name and default priority level."
        << "\nLevels: info = 0, warning = 1, error = 2"
        << std::endl;
        return false;
    }

    filename = argv[1];
    std::string arg_level = argv[2];

    std::transform(arg_level.begin(), arg_level.end(), arg_level.begin(), [](unsigned char c){
        return std::tolower(c);
    });

    if (arg_level == "0" || arg_level == "info"){
        lvl = LogLevel::INFO;
    } else if (arg_level == "1" || arg_level == "warning"){
        lvl = LogLevel::WARNING;
    } else if (arg_level == "2" || arg_level == "error"){
        lvl = LogLevel::ERROR;
    } else{
        std::cerr << "Unknown priority level" << std::endl;
        return false;
    }

    return true;
}

void LoggerConsumer(ILogger* logger){
    while (true){
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cv.wait(lock, []{ 
            return !task_queue.empty() || !is_running; 
        });

        if (task_queue.empty() && !is_running){
            break;
        }

        LogTask t = task_queue.front();
        task_queue.pop();
        lock.unlock();
        logger->Log(t.text, t.level);
    }
}

void LoggerProducer(const FileLogger& logger){
    while (true){
        std::string input_text;
        std::cout << "Enter message text" << std::endl;
        std::getline(std::cin, input_text);

        if (std::cin.eof() || input_text == "exit"){
            break;
        }

        std::string input_level_string;
        LogLevel input_level;

        std::cout << "Enter priority level.(press Enter for default level)"
        << "\nLevels: info = 0, warning = 1, error = 2" << std::endl;
        std::getline(std::cin, input_level_string);

        std::transform(input_level_string.begin(), input_level_string.end(),
            input_level_string.begin(), [](unsigned char c){
                return std::tolower(c);
        });

        if (input_level_string == "0" || input_level_string == "info"){
            input_level = LogLevel::INFO;
        } else if (input_level_string == "1" || input_level_string == "warning"){
            input_level = LogLevel::WARNING;
        } else if (input_level_string == "2" || input_level_string == "error"){
            input_level = LogLevel::ERROR;
        } else if (input_level_string == ""){
            input_level = logger.GetLevel();
        } else{
            std::cerr << "Unknown priority level" << std::endl;
            continue;
        }

        std::unique_lock<std::mutex> lock(queue_mutex);
        LogTask input_task;
        input_task.text = input_text;
        input_task.level = input_level;

        task_queue.push(input_task);
        lock.unlock();
        queue_cv.notify_one();
    }
}

int main(int argc, char** argv){
    std::string filename;
    LogLevel lvl;

    if (!ParseArgs(argc, argv, filename, lvl)){
        return 1;
    }

    FileLogger logger(filename, lvl);
    std::thread worker_thread(LoggerConsumer, &logger);

    LoggerProducer(logger);
    
    is_running = false;
    queue_cv.notify_one();
    worker_thread.join();

    return 0;
}