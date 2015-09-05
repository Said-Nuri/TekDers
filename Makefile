# C Compiler
CC=gcc

# main file
Server=server.c
Client=client.c

#executable name
EXECUTABLE_NAME=server
EXECUTABLE_NAME2=client

# library source codes
LIBRARY_SOURCES=

# library object files
LIBRARY_OBJECTS=

all:
	@${CC} ${GENERAL_PARAMS} ${Server} ${LIBRARY_OBJECTS} -o ${EXECUTABLE_NAME} -lm -pthread
	@${CC} ${GENERAL_PARAMS} ${Client} ${LIBRARY_OBJECTS} -o ${EXECUTABLE_NAME2} -lm -pthread
