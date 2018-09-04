#include <Poco/Runnable.h>
#include <Poco/Task.h>
#include <Poco/Thread.h>
#include <Poco/Timestamp.h>
#include <Poco/Util/Timer.h>
#include <Poco/Util/TimerTask.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace std;

// Poco Pthread
// class TestPthread : public Poco::Runnable {
//    public:
//     void run();
// };

// void TestPthread::run() {
//     while (1) {
//         cout << "TestPthread::run test" << endl;
//         Poco::Thread::sleep(100);
//     }
// }

// Poco Task
class TestTask : public Poco::Task {
   public:
    TestTask(std::string str) : Task(str) {}
    void runTask();
};

void TestTask::runTask() {
    while (!isCancelled()) {
        cout << "TestTask::run test" << endl;
        Poco::Thread::sleep(100);
    }
}

int main() {
    Poco::Thread pid;
    /*test Pthread*/
    // TestPthread obj;
    // pid.start(obj);

    /*test Task*/
    TestTask taskobj("123");
    pid.start(taskobj);
    sleep(4);
    taskobj.cancel();
    pid.join();
    getchar();
    return 0;
}
