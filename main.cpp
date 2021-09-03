#include <iostream>
#include <cstring>
#include <fstream>

#include "repository.h"

int main(int argc, char *argv[]) {
    Repository repo;
    if (argc == 2) {
        if (strcmp(argv[1], "init") == 0) {
            repo.init();
        }
    } else if (argc == 3) {
        if (strcmp(argv[1], "add") == 0) {
            repo.add(argv[2]);
        }
    }
}