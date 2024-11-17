cmake_minimum_required(VERSION 3.17)

project(NAT-hole-punching)

add_executable(server server.c utils.c)
add_executable(client client.c utils.c)