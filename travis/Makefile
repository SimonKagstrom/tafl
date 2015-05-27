#!/usr/bin/make

deps=crpcut-dev

.PHONY: prepare_environment

build:
	mkdir -p build build-ut
	cd build-ut && cmake ../tests/unit-tests/
	make -C build-ut

prepare_linux:
	sudo apt-add-repository -y ppa:jonlar/crpcut
	sudo apt-get update
	wget https://googlemock.googlecode.com/files/gmock-1.7.0.zip -O /tmp/gmock-1.7.0.zip
	unzip -d /tmp /tmp/gmock-1.7.0.zip
	sudo mv /tmp/gmock-1.7.0 /usr/src/gmock
	sudo apt-get install -y ${deps} git time

prepare_environment: prepare_linux
