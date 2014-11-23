CXX = g++
LD = g++
CXXFLAGS = -g -std=c++11 -c
CPPFLAGS =
LDFLAGS = -g
LDLIBS =

SRC_DIR = src
BIN_DIR = bin
EXECUTABLE = pleb

LEXER_SOURCE = $(SRC_DIR)/lexer.cpp
PARSER_SOURCE = $(SRC_DIR)/parser.cpp
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(subst $(SRC_DIR),$(BIN_DIR),$(subst .cpp,.o,$(SOURCES)))

all: $(BIN_DIR) $(BIN_DIR)/$(EXECUTABLE)

$(BIN_DIR)/$(EXECUTABLE): $(LEXER_SOURCE) $(PARSER_SOURCE) $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(LEXER_SOURCE): $(SRC_DIR)/lexer.l
	flex --outfile=$@ --header-file=$(SRC_DIR)/lexer.h $(SRC_DIR)/lexer.l

$(PARSER_SOURCE): $(SRC_DIR)/parser.y
	bison --defines=$(SRC_DIR)/parser.h --output=$@ --verbose --report-file=$(BIN_DIR)/parser.log $(SRC_DIR)/parser.y

$(BIN_DIR):
	mkdir -p $@

clean:
	rm -f $(LEXER_SOURCE) $(SRC_DIR)/lexer.h $(PARSER_SOURCE) $(SRC_DIR)/parser.h

clean_all: clean
	rm -rf $(BIN_DIR)
