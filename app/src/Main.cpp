#include <memory>
#include <MyApp.hpp>

int main(int argc, char **argv) {
    return std::make_unique<MyApp>()->run(argc, argv);
}
