#pragma once
#include "header.h"

class DataTask : public Poco::Task {
   private:
   public:
    DataTask(string name);
    ~DataTask();
};
