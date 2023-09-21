/*
Parent class for task.
*/

#pragma once
#ifndef TASK_H
#define TASK_H

class Task
{
public:
    virtual int execute(int argc, char** argv) { return -1; }
};

#endif  // TASK_H
