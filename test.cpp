
#include "include/promise.hpp"
#include <iostream>

void eventHandler(klib::KEvent event) {
    if(event.eventType == klib::EventPromiseType) {
        auto eData = std::any_cast<klib::PromiseData>(event.eventData);
        std::any value = eData.resolve.value();
            for(auto funcCallback : eData.listCallback) {
                value = funcCallback(value);
            }
    }
}

klib::KEventLoop      eventLoop(eventHandler);
klib::PromiseExecutor executor(&eventLoop);

auto asyncFunc(uint32_t num) -> klib::Promise {
    return klib::Promise(&executor, [=](klib::Resolve& resolve, klib::Reject& reject) {
        std::cout << "===================\n";
        std::cout << "==== func async ====\n";
        std::cout << "= thread id: " << std::this_thread::get_id() << std::endl;
        std::cout << "= sum " << num << std::endl;
        std::cout << "===================";
        uint32_t sum = 0;
        for(auto i = 0; i < num; i++) {
            sum += i;
        }
        resolve(sum);
    });
}

auto main() -> int {
    std::cout << "===================\n";
    std::cout << "==== main ====\n";
    std::cout << "= thread id: " << std::this_thread::get_id() << std::endl;
    std::cout << "===================";

    asyncFunc(1000000)
    .then([](std::any val) -> std::any {
        auto data = val.has_value() ? std::any_cast<uint32_t>(val) : 0;
        std::cout << "===================\n";
        std::cout << "==== then 1 ====\n";
        std::cout << "= thread id: " << std::this_thread::get_id() << std::endl;
        std::cout << "= value : " << data << std::endl;
        std::cout << "===================";
        return {};
    })
    .then([](std::any val) -> std::any {
        auto data = val.has_value() ? std::any_cast<int>(val) : 0;
        std::cout << "===================\n";
        std::cout << "==== then 2 ====\n";
        std::cout << "= thread id: " << std::this_thread::get_id() << std::endl;
        std::cout << "= value: " << data << std::endl;
        std::cout << "===================";
        return {};
    })
    .launch();

    while(1) {
        eventLoop.step();
    }

    executor.stopAll();

    return 0;
}