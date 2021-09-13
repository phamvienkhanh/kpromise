
#include "include/promise.hpp"
#include <iostream>

auto asyncFunc() -> kasync::Promise {
    return kasync::Promise([](kasync::Resolve& resolve, kasync::Reject& reject) {
        std::cout << "===================\n";
        std::cout << "==== func async ====\n";
        std::cout << "= thread id: " << std::this_thread::get_id() << std::endl;
        std::cout << "===================";
        resolve(123);
    });
}

auto main() -> int {

    kasync::PromiseExecutor::init();

    std::cout << "===================\n";
    std::cout << "==== main ====\n";
    std::cout << "= thread id: " << std::this_thread::get_id() << std::endl;
    std::cout << "===================";

    asyncFunc()
    .then([](std::any val) -> std::any {
        auto data = val.has_value() ? std::any_cast<int>(val) : 0;
        std::cout << "===================\n";
        std::cout << "==== then 1 ====\n";
        std::cout << "= thread id: " << std::this_thread::get_id() << std::endl;
        std::cout << "= value: " << data << std::endl;
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
        kasync::PromiseHandler::getIntance()->callNext();
    }

    kasync::PromiseExecutor::destroy();

    return 0;
}