#include <async-core.h>
#include <interval.h>
#include <timeout.h>
#include <repeater.h>
void AsyncCore::run() {
    Intervals::execute();
    Timeouts::execute();
    Repeaters::execute();
}