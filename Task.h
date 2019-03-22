#pragma once

#include <functional>
#include <string>

struct Task {

    Task(std::string groupName, std::function<void()> lambda,  std::function<void()> completion = []() { }) {

        group = groupName;
        func  = lambda;

        completionCallback = completion;
    }

    std::string           group;
    std::function<void()> func;
    std::function<void()> completionCallback;

};