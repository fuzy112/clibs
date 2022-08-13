cmake_minimum_required(VERSION 3.4)

project(clibs)

add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)

add_executable(avl2dot avltree.c avltree2dot.c tree2dot.c)

add_executable(rb2dot rbtree.c rbtree2dot.c tree2dot.c)

add_executable(tree-benchmark tree-benchmark.cpp)

add_library(tools OBJECT dict.c quark.c rbtree.c avltree.c splay.c)
