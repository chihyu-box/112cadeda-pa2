# Compiler, Flags, and Libraries
CXX = g++
CXXFLAGS = -Wall -std=c++17 -I./glpk -I./gurobi
#LDFLAGS = -L/usr/local/lib -lglpk -Wl,-rpath=/usr/local/lib
LDFLAGS = ./libglpk.a ./libgurobi_c++.a ./libgurobi100.so

# Directories
SRCDIR = src
OBJDIR = obj
BINDIR = exec

# Files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))
TARGET = $(BINDIR)/M11207432

# Rules
all: directories $(TARGET)

directories:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all directories clean
