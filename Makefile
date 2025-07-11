CXX = g++
CPPFLAGS = -Wall -O3 -std=c++14 -g
LDFLAGS = -lm

TARGET = merge_add
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CXX) $(CPPFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

%.d: %.cpp
	@set -e; \
	$(CC) -M $(CFLAGS) $< > $@.tmp; \
	sed 's,\($(notdir $*)\)\.o[ :]*,\1.o $@ : ,g' < $@.tmp > $@; \
	rm -f $@.tmp

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)

.PHONY: all clean
