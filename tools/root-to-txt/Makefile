MEAS_PATH := ../../../sample-measurements/downloaded/HitsSave-pu200-with-layers.root

R_LDFLAGS := $(shell root-config --ldflags)
R_LIBS := $(shell root-config --libs)
R_CFLAGS := $(shell root-config --cflags)
R_ALL := $(R_LDFLAGS) $(R_LIBS) $(R_CFLAGS)

SRC := src/*.cpp

app: $(SRC)
	g++ -Wall -fPIC $(R_ALL) $^ -o $@

run: app
	./$^ $(MEAS_PATH) 'htt;1'

clean:
	rm -rf *.o *.txt app

.PHONY: clean run
