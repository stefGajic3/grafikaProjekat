#include <MyApp.hpp>
#include <memory>

int main(int argc, char **argv) {
    return std::make_unique<MyApp>()->run(argc, argv);
}
