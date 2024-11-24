#include <spdlog/spdlog.h>
#include <iostream>
#include <memory>
#include <vector>
#include "./data/TemperatureData.pb.h"
#include "./include/node.h"
#include "include/topic.hpp"
#include "spdlog/sinks/stdout_sinks.h"
int main()
{
    spdlog::set_level(spdlog::level::debug);  // 设置最低日志级别为 debug
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%l---%$] [thread %t] [source %s] [function %!] [line %#] %v");
    spdlog::stdout_logger_mt("console");
    // SPDLOG_DEBUG("sdfasdf{}",1);

    Node node;
    node.init();
    TemperatureData data;
    data.set_location("TestLocation");
    data.set_temperature(25.5);
    std::shared_ptr<Topic> topic = std::make_shared<Topic>("/topic1");
    topic->setData(1, data);
    node.create_publish_topic("/topic1", topic);

    // std::vector<uint8_t> bytes = {'a', 'b', 'c'};
    // int result;
    // for (int i = 0; i < 10000000; i++)
    // {
    //     result = node.multiacast(bytes);
    // }
    while (true)
    {}
    return 0;
}
