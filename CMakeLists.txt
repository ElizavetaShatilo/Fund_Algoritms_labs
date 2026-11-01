cmake_minimum_required(VERSION 3.20)
project(Fund_Algorithms_labs C)

set(CMAKE_C_STANDARD 11)

add_executable(task1 task1/main.c task1/parsing.c task1/strings.c)

add_executable(task2 task2/main.c task2/employees.c task2/parsing.c)

add_executable(task3 task3/main.c task3/search.c)

add_executable(task4 task4/main.c task4/knapsack.c task4/parsing.c)

add_executable(task5 task5/main.c task5/convert.c task5/overprint.c task5/roman.c task5/zeckendorf.c)

add_executable(task6 task6/main.c task6/convert.c task6/overscan.c task6/roman.c task6/zeckendorf.c)

add_executable(task7 task7/main.c task7/vec_ops.c)

add_executable(task8 task8/main.c task8/radix_add.c)

add_executable(task9 task9/main.c task9/frac_base.c)

add_executable(task10 task10/main.c task10/poly_reexp.c)