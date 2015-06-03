#!/usr/bin/make

deps=crpcut-dev g++-4.8 git time

.PHONY: prepare_environment

build:
	mkdir -p build build-ut
	cd build-ut && cmake ../tests/unit-tests/
	make -C build-ut
	cd build-ut && kcov --coveralls-id=$TRAVIS_JOB_ID --include-pattern=tafl ./ut

prepare_linux:
	sudo apt-add-repository -y ppa:jonlar/crpcut
	sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
	sudo apt-get update -y
	sudo apt-get install -y ${deps}
	sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 90
	wget https://googlemock.googlecode.com/files/gmock-1.7.0.zip -O /tmp/gmock-1.7.0.zip
	unzip -d /tmp /tmp/gmock-1.7.0.zip
	cd /tmp/gmock-1.7.0 && ./configure && make
	cd /tmp/gmock-1.7.0/gtest && ./configure && make
	sudo mv /tmp/gmock-1.7.0 /usr/src/gmock
	wget https://github.com/SimonKagstrom/kcov/archive/master.tar.gz && tar xzf master.tar.gz && mkdir kcov-master/build && cd kcov-master/build && cmake .. && make && sudo make install

prepare_environment: prepare_linux
