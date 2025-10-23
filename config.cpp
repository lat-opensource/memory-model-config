// SPDX-License-Identifier: GPL-2.0

#include <array>
#include <complex.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>
using namespace std;

enum CONFIG_TYPE {
    TSO,
    STFILL,
};

bool exit_ = false;

string exec(const char* cmd)
{
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

    if (!pipe) {
        throw runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}
string get_config_name(CONFIG_TYPE which)
{
    switch (which) {
    case TSO:
        return "tso_disabled";
        break;
    case STFILL:
        return "stfill";
        break;
    default:
        return "";
        break;
    }
    return "";
}

void show_value(CONFIG_TYPE which)
{
    string config_name = get_config_name(which);
    string Command = "cat /sys/kernel/loongarch_csr/" + config_name;
    try {
        string output = exec(Command.c_str());
        cout << config_name << ":" << output << endl;
    } catch (const runtime_error& e) {
        cerr << "get value failed: " << e.what() << endl;
    }
}
void set_value(CONFIG_TYPE which, string value)
{
    string config_name = get_config_name(which);
    // echo 0  > /sys/kernel/loongarch_csr/tso_disabled

    string Command = "su -c 'echo " + value + " > /sys/kernel/loongarch_csr/" + config_name + "'";
    try {
        string output = exec(Command.c_str());
        cout << "success." << endl
             << "new value:" << endl;
        show_value(which);
    } catch (const runtime_error& e) {
        cerr << "failed: " << e.what() << endl;
    }
}

void print_help()
{
    cout << endl
         << "Usage:" << endl;
    cout << "   - sudo ./config -h" << endl;
    cout << "   - sudo ./config tso enable" << endl;
    cout << "   - sudo ./config tso disable" << endl;
    cout << "   - sudo ./config tso show" << endl;
    cout << "   - sudo ./config stfill enable" << endl;
    cout << "   - sudo ./config stfill disable" << endl;
    cout << "   - sudo ./config stfill show" << endl;
}
int main(int argv, char* argc[])
{

    if (argv == 2 && strcmp(argc[1], "-h") == 0) {
        print_help();
        return 0;
    }

    if (argv != 3) {
        cout << "please input 2 args" << endl;
        print_help();
        return 0;
    }
    if (geteuid() != 0) {
        cout << "Please rerun the program with 'sudo'." << endl;
        print_help();
        return 0;
    }
    string config_which = argc[1];
    string config_operation = argc[2];

    if (config_which == "tso") {
        if (config_operation == "show") {
            show_value(TSO);
        } else if (config_operation == "enable") {
            set_value(TSO, "0");
        } else if (config_operation == "disable") {
            set_value(TSO, "1");

        } else {
            cout << "please input 'show', 'disable' or 'enable'." << endl;
        }
    } else if (config_which == "stfill") {
        if (config_operation == "show") {
            show_value(STFILL);
        } else if (config_operation == "enable") {
            set_value(STFILL, "1");
        } else if (config_operation == "disable") {
            set_value(STFILL, "0");
        } else {
            cout << "please input 'show', 'disable' or 'enable'." << endl;
        }
    } else {
        cout << "please input correct config name." << endl;
        print_help();
        return 0;
    }

    return 0;
}
