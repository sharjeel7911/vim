# Variables
CXX = g++
CXXFLAGS = -std=c++17 -O3
TARGET = shar
INSTALL_DIR = /usr/local/bin

# Default build rule
all:
	$(CXX) $(CXXFLAGS) *.cpp -o $(TARGET)

# The "magic" step for you and your friend
install: all
	@echo "Installing $(TARGET) globally into $(INSTALL_DIR)..."
	sudo cp $(TARGET) $(INSTALL_DIR)/$(TARGET)
	@echo "Done! You can now use '$(TARGET)' from any folder on your machine."

# Clean up build binaries from project folder
clean:
	rm -f $(TARGET)