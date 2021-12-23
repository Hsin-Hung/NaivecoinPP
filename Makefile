CXX       := clang++
CXX_FLAGS := -std=c++17 #-Wall -Wextra 

BIN     := bin
SRC     := src
LIBRARIES   := -stdlib=libc++
EXECUTABLE  := main


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	@echo "🚀 Executing..."
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/main.cpp $(SRC)/*/*.cpp
	@echo "🚧 Building..."
	$(CXX) $(CXX_FLAGS) -I$(SRC) $(LIBRARIES) -g $^ -o $@

clean:
	@echo "🧹 Clearing..."
	-rm $(BIN)/*