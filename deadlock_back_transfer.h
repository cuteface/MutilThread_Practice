//
// Created by 王宇昆 on 2023/12/23.
//

#ifndef DEV_DEADLOCK_BACK_TRANSFER_H
#define DEV_DEADLOCK_BACK_TRANSFER_H

#include <set>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

class deadlock_back_transfer {

};

class Account {
public:
    Account(string name, double money): mName(name), mMoney(money) {}

public:
    void changeMoney(double amount) {
        unique_lock<mutex> lock(mMoneyLock);
        mCondition_variable.wait(lock, [this, amount] {
           return mMoney + amount > 0;
        });
        mMoney += amount;
        mCondition_variable.notify_all();
    }
    string getName() {
        return mName;
    }
    double getMoney() {
        return mMoney;
    }
    mutex* getLock() {
        return &mMoneyLock;
    }

private:
    string mName;
    double mMoney;
    mutex mMoneyLock;
    condition_variable mCondition_variable;
};

class Bank {
public:
    void addAccount(Account* account) {
        mAccounts.insert(account);
    }

    bool transferMoney(Account* accountA, Account* accountB, double amount) {
        lock(*accountA->getLock(), *accountB->getLock());
        lock_guard<mutex>guardA(*accountA->getLock(), adopt_lock);
        lock_guard<mutex>guardB(*accountB->getLock(), adopt_lock);

        if (amount > accountA->getMoney()) {
            return false;
        }

        accountA->changeMoney(-amount);
        accountB->changeMoney(amount);
        return true;
    }

    void transferMoney1(Account* accountA, Account* accountB, double amount) {
        accountA->changeMoney(-amount);
        accountA->changeMoney(amount);
    }

    double totalMoney() const {
        double sum = 0;
        for (auto a: mAccounts) {
            sum += a->getMoney();
        }
        return sum;
    }


private:
    set<Account*> mAccounts;
};

#endif //DEV_DEADLOCK_BACK_TRANSFER_H
