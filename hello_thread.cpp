//
// Created by 王宇昆 on 2023/12/13.
//
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
#include <iomanip>
#include <future>
#include "deadlock_back_transfer.h"

using namespace std;

void hello() {
    cout << "Hello from new thread." <<endl;
}

void hello_new(std::string name) {
    cout << "Hello from " << name << "." <<endl;
}

// thread_self_manage start
void print_time() {
    auto now = chrono::system_clock::now();
    auto in_time_t = chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << put_time(localtime(&in_time_t), "%Y-%M-%D %X");
    cout << "now is: " << ss.str() <<endl;
}

void sleep_thread() {
    this_thread::sleep_for(chrono::seconds(3));
    cout << "[thread-" << this_thread::get_id() << "] is waking up." << endl;
}

void loop_thread() {
    for (int i = 0; i < 10 ; ++i) {
        cout << "[thread-" << this_thread::get_id() << "] print: " << i <<endl;
    }
}

// thread_self_manage end

// one thread cacl sqrt sum
static const int MAX = 10e8;
static double sum = 0;
static mutex exclusive;

void worker(int min, int max) {
    for (int i = min; i <= max; i++) {
        sum += sqrt(i);
    }
}

void concurrent_worker(int min, int max) {
    double tmp_sum = 0;
    for (int i = min; i <= max; i++) {
        tmp_sum += sqrt(i);
    }
    exclusive.lock();
    sum += tmp_sum;
    exclusive.unlock();
}

void serial_task(int min, int max) {
    auto start_time = chrono::steady_clock::now();
    sum = 0;
    worker(0, max);
    auto end_time = chrono::steady_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    cout << "Serial task finish, " << ms << "ms consumed, Result: " << sum << endl;
}

void concurrent_task(int min, int max) {
    auto start_time = chrono::steady_clock::now();

    unsigned concurrent_count = thread::hardware_concurrency();
    cout << "hardware_concurrency: " << concurrent_count << endl;
    vector<thread> threads;
    min = 0;
    sum = 0;
    for(int t = 0; t < concurrent_count; t++) {
        int range = max / concurrent_count * (t + 1);
        //threads.push_back(thread(worker, min, range));
        threads.push_back(thread(concurrent_worker, min, range));
        min = range + 1;
    }
    for (auto& t: threads) {
        t.join();
    }

    auto end_time = chrono::steady_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    cout << "Concurrent task finish, " << ms << " ms consumed. Result: " << sum << endl;
}

static mutex sCoutLock;
void randomTransfer(Bank* bank, Account* accountA, Account* accountB) {
    while(true) {
        double randomMoney = ((double)rand() / RAND_MAX) * 100;
        if (bank->transferMoney(accountA, accountB, randomMoney)) {
            sCoutLock.lock();
            cout << "Transfer" << randomMoney << " from" << accountA->getName()
            << " to " << accountB->getName() << ", Bank totalMoney: " << bank->totalMoney() << endl;
            sCoutLock.unlock();
        } else {
            sCoutLock.lock();
            cout << "Transfer failed, " << accountA->getName() << " has only $" << accountA->getMoney()
            << " , but " << randomMoney << " required" << endl;
            sCoutLock.unlock();
        }
    }
}

void randomTransfer1(Bank* bank, Account* accountA, Account* accountB) {
    while (true) {
        double randomMoney = ((double)rand() / RAND_MAX) * 100;
        {
            lock_guard<mutex> guard(sCoutLock);
            cout << "Try to Transfer " << randomMoney
                 << " from " << accountA->getName() << "(" << accountA->getMoney()
                 << ") to " << accountB->getName() << "(" << accountB->getMoney()
                 << "), Bank totalMoney: " << bank->totalMoney() << endl;
        }
        bank->transferMoney1(accountA, accountB, randomMoney);
    }
}

int main() {
    // part I thread api
//    thread t1(hello);
//    cout<< "t1 joinable: "<< t1.joinable() << endl;
//
//    thread t2([] {
//        cout << "Hello from thread 2" <<endl;
//    });
//
//    thread t3(hello_new, "thread 3");
//
//    t1.join();
//    t2.join();
//    t3.join();
//
//    print_time();
//    thread sleep_t(sleep_thread);
//    thread loop_t(loop_thread);
//    sleep_t.join();
//    //loop_t.join();
//    loop_t.detach();
//    print_time();

    // one thread
    //serial_task(0, MAX);

    // mutil thread
    //concurrent_task(0, MAX);

//    Account a("yukun", 100);
//    Account b("xiaofang", 100);
//
//    Bank aBank;
//    aBank.addAccount(&a);
//    aBank.addAccount(&b);
//
//    thread t1(randomTransfer1, &aBank, &a, &b);
//    thread t2(randomTransfer1, &aBank, &b, &a);
//
//    t1.join();
//    t2.join();

    // async task
    double result = 0;
    cout << "Async task with lambda triggered, thread " << this_thread::get_id() << endl;
    auto f2 = async(launch::async, [&result]() {
        cout << "Lambda task in thread: " << this_thread::get_id() << endl;
        for (int i = 0; i <= MAX; i++) {
            result += sqrt(i);
        }
    });
    f2.wait();
    cout << "Async task with lambda finish, result: " << result << endl;

    return 0;
}
