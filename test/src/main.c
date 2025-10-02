#include <naxa/naxa.h>

int main(int argc, char** argv) {
    naxa_init();
    naxa_log("TEST");
    naxa_teardown();
}