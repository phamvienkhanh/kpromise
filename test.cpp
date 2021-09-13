
#include "include/promise.hpp"
#include <iostream>

auto main() -> int {

    kasync::PromiseExecutor::init();

    kasync::Promise p([](kasync::Resolve& resolve, kasync::Reject& reject) {
        std::any a = 123;
        resolve(a);
    });
    p.then([](std::any val) -> std::any {
        std::cout << "==== then ====\n"; 
        std::cout << std::any_cast<int>(val) << std::endl;
        return {};
    });
    p.launch();

    while(1) {
        kasync::PromiseHandler::getIntance()->callNext();
    }

    kasync::PromiseExecutor::destroy();

    return 0;
}