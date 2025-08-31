CXX = g++
CPPFLAGS = -Wall -O3 -std=c++14 -g -MMD -MP -mcmodel=medium
LDFLAGS = -lm

BUILD_DIR = build

TARGET = $(BUILD_DIR)/merge_add
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(SRCS:%.cpp=$(BUILD_DIR)/%.d)

all: $(TARGET)

-include $(DEPS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
