
CPP=g++

FLAGS:=-Wall -Wextra

SRC:=$(wildcard *.cpp)

OBJECTS:=$(SRC:.cpp=.o)


BUILD_DIR:=build
OBJECTS:=$(addprefix $(BUILD_DIR)/,$(OBJECTS))

TARGET:=main
 
all: clean $(TARGET)

$(TARGET):$(OBJECTS)
			$(CPP) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o:%.cpp
			mkdir -p $(BUILD_DIR)
			$(CPP) $(FLAGS) -c $< -o $@


clean:
	rm -rf $(BUILD_DIR) $(TARGET)