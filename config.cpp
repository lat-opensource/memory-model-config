// SPDX-License-Identifier: GPL-2.0

#include <array>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>
using namespace std;

static const char SYSFS_PATH[] = "/sys/kernel/loongarch_csr/memory_model";

static const char* mode_names[] = {"weak", "store", "tso"};

string exec(const char* cmd)
{
    array<char, 128> buffer;
    string result;

    struct PipeCloser {
        void operator()(FILE* f) const { pclose(f); }
    };
    unique_ptr<FILE, PipeCloser> pipe(popen(cmd, "r"));

    if (!pipe)
        throw runtime_error("popen() failed!");

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();

    return result;
}

static int read_mode()
{
    try {
        string s = exec((string("cat ") + SYSFS_PATH).c_str());
        return s[0] - '0';
    } catch (const runtime_error& e) {
        cerr << "read failed: " << e.what() << endl;
        return -1;
    }
}

static bool set_mode(int val)
{
    string cmd = "su -c 'echo " + to_string(val) + " > " + SYSFS_PATH + "'";
    try {
        exec(cmd.c_str());
        return (read_mode() == val);
    } catch (const runtime_error& e) {
        cerr << "write failed: " << e.what() << endl;
        return false;
    }
}

static void show_status()
{
    int m = read_mode();
    if (m >= 0 && m <= 2)
        cout << "\n  current: " << mode_names[m] << " (" << m << ")\n";
    else
        cout << "\n  unable to read (run as root, insmod first?)\n";
}

int main()
{
    if (geteuid() != 0) {
        cerr << "Please run with sudo.\n";
        return 1;
    }

    cout << "LoongArch Memory Model Config\n"
         << "==============================\n";

    while (true) {
        show_status();
        cout << "\n"
             << "  [0] weak    (no ordering)\n"
             << "  [1] store   (store-store ordered)\n"
             << "  [2] tso     (store + load ordered)\n"
             << "  [q] quit\n"
             << "\n"
             << "  choice: ";
        cout.flush();

        string input;
        getline(cin, input);

        if (input.empty())
            continue;
        char c = input[0];
        if (c == 'q' || c == 'Q')
            break;
        if (c >= '0' && c <= '2') {
            int val = c - '0';
            cout << "  switching to " << mode_names[val] << "... ";
            cout.flush();
            if (set_mode(val))
                cout << "OK\n";
            else
                cout << "FAIL\n";
        }
    }

    cout << "\nbye\n";
    return 0;
}
