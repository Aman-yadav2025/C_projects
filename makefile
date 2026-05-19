# CC = gcc
CXX = g++
# CFLAGS = -Wall -I./includes
CXXFLAGS = -Wall -I./includes
TARGET = my_app

# 1. Ensure these match your actual files on disk
SRCS = main.cpp src/math_utils.cpp

# 2. Convert .c to .o
# OBJS = $(SRCS:.c=.o)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET) tidy
# all: $(TARGET) use if you want object files to remain

$(TARGET): $(OBJS)
# 	$(CC) $(OBJS) -o $(TARGET)
	$(CXX) $(OBJS) -o $(TARGET)

# 3. Rule must look for .c files if your SRCS are .c
# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Removes only the object files, keeps the executable
tidy:
	rm -f $(OBJS)

# Removes everything (standard practice)
clean:
	rm -f $(OBJS) $(TARGET)