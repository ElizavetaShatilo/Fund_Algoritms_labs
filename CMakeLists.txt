cmake_minimum_required(VERSION 3.20)
project(Fund_Algorithms_labs C)

set(CMAKE_C_STANDARD 11)

add_executable(task1 task1/main.c task1/parsing.c task1/strings.c)

add_executable(task2 task2/main.c task2/employees.c task2/parsing.c)

add_executable(task3 task3/main.c task3/search.c)

add_executable(task4 task4/main.c task4/knapsack.c task4/parsing.c)