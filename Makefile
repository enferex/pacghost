APP = pacghost
CXXFLAGS = -pedantic -std=c++17 -Wall

all: release

release: CXXFLAGS += -O3 -g0
release: $(APP)

debug: CXXFLAGS += -O0 -g3
debug: $(APP)

$(APP): main.cc
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	$(RM) $(APP)
