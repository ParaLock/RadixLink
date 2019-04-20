#pragma once

#include <vector>
#include <iostream>

struct Buffer {
	
	Buffer() {

		isLast = false;
		seq = 0;
		//count = 0;
	}

	Buffer(size_t size) : data(size) {

		isLast = false;
		seq    = 0;
		//count = 0;
		//count = size;
	}

	~Buffer() {	}

	void write(char* src, unsigned int size, bool incPos = true) {
		
		unsigned int count = 0;
		// data.resize(data.size() + size);

		// memcpy(getBase() + count, src, size);
		// count += size;

		for(int i = 0; i < size; i++) {
		
			data.push_back(src[count]);
			
			if(incPos)
				count++;
		}
	}
	
	unsigned int getSize() {
		
		return data.size();
	}
	
	char * getBase() {
		
		if(data.size() == 0) {
			return nullptr;
		}

		return &data[0];
	}
	
	void clear() {

		data.clear();
	}

	void resize(size_t num) {

		data.resize(num);
	}

	char* getCurrentOffset() {
	
		return &data[0] + data.size();
	}
	
	void print() {

		for(int i = 0; i < data.size(); i++) {

			std::cout << data[i];
		}
		
		std::cout << std::endl;
	}
	
	bool         	  isLast;
	unsigned int 	  seq;
	//size_t  		  count;
	//size_t       	  size;
	std::vector<char> data;
};
