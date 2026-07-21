# Variables
CXX = g++
CXXFLAGS = -std=c++17 -O3
TARGET = shar
INSTALL_DIR = /usr/local/bin

# Default build rule
all:
	$(CXX) $(CXXFLAGS) src/*.cpp -o $(TARGET)


install: all
	@echo "Installing $(TARGET) globally into $(INSTALL_DIR)..."
	sudo cp $(TARGET) $(INSTALL_DIR)/$(TARGET)
	@echo "Done! You can now use '$(TARGET)' editor from any folder on your machine."

# Clean up build binaries from project folder
clean:
	rm -f $(TARGET)	
