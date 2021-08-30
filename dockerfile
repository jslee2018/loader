FROM ubuntu:18.04

RUN apt-get upgrade
RUN apt-get update
RUN apt-get install -y vim git gcc gdb wget python