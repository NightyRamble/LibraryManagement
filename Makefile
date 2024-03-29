
CXX = g++

# Define the source files
SOURCES = client.cpp serverM.cpp serverS.cpp serverL.cpp serverH.cpp

# Define any compile-time flags
CXXFLAGS = -Wall -g -std=c++11


# Define the object files (one for each source)
OBJECTS = $(SOURCES:.cpp=.o)

# Define the executable files (one for each source)
EXECUTABLES = $(SOURCES:.cpp=)

# Default target
all: $(EXECUTABLES)

# Individual targets for each executable
client: client.o
	$(CXX) $(CXXFLAGS) client.o -o client

serverM: serverM.o
	$(CXX) $(CXXFLAGS) serverM.o -o serverM

serverS: serverS.o
	$(CXX) $(CXXFLAGS) serverS.o -o serverS

serverL: serverL.o
	$(CXX) $(CXXFLAGS) serverL.o -o serverL

serverH: serverH.o
	$(CXX) $(CXXFLAGS) serverH.o -o serverH

# Generic rule for object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(EXECUTABLES)